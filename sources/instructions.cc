#include <cstring>
#include <iostream>
#include <unistd.h>

#include "instructions.hh"
#include "decoder.hh"
#include "rv32i_behaviour.generated.hh"
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
handleEnvironmentCall( CPUModel& cpu_model)
{
    uint32_t a7 = cpu_model.getX( abi::AbiRegisterName::a7); // Syscall number
    SupportedSyscalls call = static_cast<SupportedSyscalls>( a7);
    switch ( call )
    {
        case SupportedSyscalls::write:
        {
            uint32_t fd = cpu_model.getX( abi::AbiRegisterName::a0);
            uint32_t buf = cpu_model.getX( abi::AbiRegisterName::a1);
            uint32_t count = cpu_model.getX( abi::AbiRegisterName::a2);
            RVI_DEBUG( "Syscall: write( fd={}, buf={}, count={})", fd, buf, count);

            int host_fd;
            {
                auto host_fd_r = toHostFd( static_cast<int>( fd));
                if ( !host_fd_r.ok() )
                {
                    return host_fd_r.error();
                }
                host_fd = host_fd_r.value();
            }

            std::vector<std::byte> write_buffer;
            write_buffer.reserve( count);
            // TODO make this cleaner to allow memory access checks
            cpu_model.memory().loadTrusted( buf, count, write_buffer.data());
            ssize_t write_r = ::write( host_fd, write_buffer.data(), count);
            cpu_model.setX( abi::AbiRegisterName::a0, static_cast<uint32_t>( write_r));
            return RunAction( Continue());
        }
        case SupportedSyscalls::read:
        {
            uint32_t fd = cpu_model.getX( abi::AbiRegisterName::a0);
            uint32_t buf = cpu_model.getX( abi::AbiRegisterName::a1);
            uint32_t count = cpu_model.getX( abi::AbiRegisterName::a2);
            RVI_DEBUG( "Syscall: read( fd={}, buf={}, count={})", fd, buf, count);

            int host_fd;
            {
                auto host_fd_r = toHostFd( static_cast<int>( fd));
                if ( !host_fd_r.ok() )
                {
                    return host_fd_r.error();
                }
                host_fd = host_fd_r.value();
            }

            std::vector<std::byte> read_buffer;
            read_buffer.reserve( count);
            ssize_t read_r = ::read( host_fd, read_buffer.data(), count);

            if ( read_r >= 0 )
            {
                cpu_model.memory().storeTrusted( buf, read_r, read_buffer.data());
            }
            cpu_model.setX( abi::AbiRegisterName::a0, static_cast<uint32_t>( read_r));
            return RunAction( Continue());
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

        return Error( ErrorCode::UnknownInstruction,
                      std::format( "Unknown instruction: {}", bits));
    }

    auto r = runGeneratedInstruction( cpu_model, encoding->instruction, bits);
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
