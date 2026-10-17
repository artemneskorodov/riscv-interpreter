#include <cstring>
#include <iostream>
#include <unistd.h>

#include "instructions.hh"
#include "decoder.hh"
#include "insn/execute.hh"
#include "debug.hh"
#include "abi.hh"

namespace riscv
{
namespace insn
{

enum class SupportedSyscalls
{
    write = 64,
    read = 63,
    exit = 93,
};

Expected<int>
toHostFd( int guest_fd)
{
    switch ( guest_fd )
    {
        case 0: return STDIN_FILENO;
        case 1: return STDOUT_FILENO;
        case 2: return STDERR_FILENO;
        default:
        {
            RVI_FAILURE( "Unsupported guest file descriptor: {}", guest_fd);
        }
    }
}

Expected<RunAction>
handleSyscallWrite( CPUModel& cpu_model)
{
    uint32_t fd = cpu_model.getX( abi::AbiRegisterName::a0);
    uint32_t buf = cpu_model.getX( abi::AbiRegisterName::a1);
    uint32_t count = cpu_model.getX( abi::AbiRegisterName::a2);
    RVI_DEBUG( "Syscall: write( fd={}, buf={}, count={})", fd, buf, count);

    //
    // Translating guest fd to host fd
    // TODO: incorrect fd must be interpreted as programmer error, so the return value of syscall
    //       must be set to a0
    //
    int host_fd;
    {
        auto r = toHostFd( static_cast<int>( fd));
        RVI_ASSERT( r.ok());
        host_fd = r.value();
    }

    //
    // Loading memory from guest
    //
    std::vector<std::byte> host_buffer;
    {
        host_buffer.resize( count);
        auto r = cpu_model.memory().copyFromGuest( buf, host_buffer.data(), count);
        if ( !r.ok() )
        {
            return r.error();
        }
    }

    //
    // Running host write
    //
    {
        ssize_t write_r = ::write( host_fd, host_buffer.data(), count);
        cpu_model.setX( abi::AbiRegisterName::a0, static_cast<uint32_t>( write_r));
    }

    return RunAction( Continue());
}

Expected<RunAction>
handleSyscallRead( CPUModel& cpu_model)
{
    uint32_t fd = cpu_model.getX( abi::AbiRegisterName::a0);
    uint32_t buf = cpu_model.getX( abi::AbiRegisterName::a1);
    uint32_t count = cpu_model.getX( abi::AbiRegisterName::a2);
    RVI_DEBUG( "Syscall: read( fd={}, buf={}, count={})", fd, buf, count);

    //
    // Translating guest fd to host fd
    // TODO: incorrect fd must be interpreted as programmer error, so the return value of syscall
    //       must be set to a0
    //
    int host_fd;
    {
        auto r = toHostFd( static_cast<int>( fd));
        RVI_ASSERT( r.ok());
        host_fd = r.value();
    }

    //
    // Creating host buffer
    //
    std::vector<std::byte> host_buffer( count);

    //
    // Running host read
    //
    int32_t result;
    {
        ssize_t read_r = ::read( host_fd, host_buffer.data(), count);
        result = static_cast<int32_t>( read_r);
        cpu_model.setX( abi::AbiRegisterName::a0, result);
    }

    //
    // Loading to guest memory
    //
    {
        if ( result > 0 )
        {
            auto r = cpu_model.memory().copyToGuest( buf, host_buffer.data(), result);
            if ( !r.ok() )
            {
                return r.error();
            }
        }
    }

    return RunAction( Continue());
}

Expected<RunAction>
handleEnvironmentCall( CPUModel& cpu_model)
{
    uint32_t a7 = cpu_model.getX( abi::AbiRegisterName::a7); // Syscall number
    SupportedSyscalls call = static_cast<SupportedSyscalls>( a7);
    switch ( call )
    {
        case SupportedSyscalls::write:
        {
            return handleSyscallWrite( cpu_model);
        }
        case SupportedSyscalls::read:
        {
            return handleSyscallRead( cpu_model);
        }
        case SupportedSyscalls::exit:
        {
            uint32_t status = cpu_model.getX( abi::AbiRegisterName::a0);
            RVI_DEBUG( "Syscall: exit( status={})", static_cast<int>( status));
            return RunAction( Exit( static_cast<int>( status)));
        }
        default:
        {
            RVI_FAILURE( "Unsupported syscall: {}", a7);
        }
    }
}

Expected<RunAction>
handleStepResult( CPUModel& cpu_model,
                  StepResult result)
{
    switch ( result )
    {
        case StepResult::Continue:
        {
            return RunAction( Continue());
        }
        case StepResult::Breakpoint:
        {
            RVI_FAILURE( "Breakpoint exceptions are not supported");
        }
        case StepResult::EnvironmentCall:
        {
            return handleEnvironmentCall( cpu_model);
        }
        default:
        {
            RVI_UNREACHABLE();
        }
    }
}

Expected<RunAction>
runSingleInstruction( CPUModel& cpu_model)
{
    auto load_bits = cpu_model.memory().loadExecutable( cpu_model.getPC());
    if ( !load_bits.ok() )
    {
        RVI_DEBUG( "Error while loading instruction: pc = {}", cpu_model.getPC());
        return load_bits.error();
    }

    isa::InstructionBits bits = load_bits.value();

    auto encoding = isa::matchInstruction( bits);
    if ( !encoding.has_value() )
    {
        RVI_DEBUG( "Error while matching instruction: pc = {:#x}, bits = {:#x}",
                   cpu_model.getPC(),
                   bits);

        return Error( ErrorCode::InvalidInstruction,
                      std::format( "Unknown instruction: {}", bits));
    }

    auto r = execute( cpu_model, bits, encoding->instruction);
    if ( !r.ok() )
    {
        RVI_DEBUG( "Error while running instruction: pc = {:#x}, bits = {:#x}, instruction = {}",
                   cpu_model.getPC(),
                   bits,
                   encoding->name);
        return r.error();
    }
    auto step_r = handleStepResult( cpu_model, r.value());
    return step_r;
}

} // ! namespace insn
} // ! namespace riscv
