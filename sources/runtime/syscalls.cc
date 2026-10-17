#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include <vector>

#include "runtime/syscalls.hh"
#include "isa/registers.hh"
#include "helpers/diagnostics.hh"

namespace rvi
{
namespace runtime
{

namespace
{

enum class SupportedSyscall
{
    Write = 64,
    Read = 63,
    Exit = 93,
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

Expected<interpreter::RunAction>
handleWrite( CPUModel& cpu_model)
{
    uint32_t fd = cpu_model.getX( isa::RegisterName::a0);
    uint32_t buf = cpu_model.getX( isa::RegisterName::a1);
    uint32_t count = cpu_model.getX( isa::RegisterName::a2);
    RVI_DEBUG( "Syscall: write( fd={}, buf={}, count={})", fd, buf, count);

    auto host_fd = toHostFd( static_cast<int>( fd));
    if ( !host_fd.ok() )
    {
        return host_fd.error();
    }

    std::vector<std::byte> host_buffer( count);
    auto copy_result = cpu_model.memory().copyFromGuest( buf, host_buffer.data(), count);
    if ( !copy_result.ok() )
    {
        return copy_result.error();
    }

    ssize_t write_result = ::write( host_fd.value(), host_buffer.data(), count);
    cpu_model.setX( isa::RegisterName::a0, static_cast<uint32_t>( write_result));
    return interpreter::RunAction( interpreter::Continue());
}

Expected<interpreter::RunAction>
handleRead( CPUModel& cpu_model)
{
    uint32_t fd = cpu_model.getX( isa::RegisterName::a0);
    uint32_t buf = cpu_model.getX( isa::RegisterName::a1);
    uint32_t count = cpu_model.getX( isa::RegisterName::a2);
    RVI_DEBUG( "Syscall: read( fd={}, buf={}, count={})", fd, buf, count);

    auto host_fd = toHostFd( static_cast<int>( fd));
    if ( !host_fd.ok() )
    {
        return host_fd.error();
    }

    std::vector<std::byte> host_buffer( count);
    ssize_t read_result = ::read( host_fd.value(), host_buffer.data(), count);
    cpu_model.setX( isa::RegisterName::a0, static_cast<uint32_t>( read_result));

    if ( read_result > 0 )
    {
        auto copy_result = cpu_model.memory().copyToGuest(
            buf, host_buffer.data(), static_cast<uint32_t>( read_result));
        if ( !copy_result.ok() )
        {
            return copy_result.error();
        }
    }

    return interpreter::RunAction( interpreter::Continue());
}

} // ! anonymous namespace

Expected<interpreter::RunAction>
handleEnvironmentCall( CPUModel& cpu_model)
{
    uint32_t number = cpu_model.getX( isa::RegisterName::a7);
    switch ( static_cast<SupportedSyscall>( number) )
    {
        case SupportedSyscall::Write:
            return handleWrite( cpu_model);
        case SupportedSyscall::Read:
            return handleRead( cpu_model);
        case SupportedSyscall::Exit:
        {
            uint32_t status = cpu_model.getX( isa::RegisterName::a0);
            RVI_DEBUG( "Syscall: exit( status={})", static_cast<int>( status));
            return interpreter::RunAction( interpreter::Exit( static_cast<int>( status)));
        }
        default:
        {
            RVI_FAILURE( "Unsupported syscall: {}", number);
        }
    }
}

} // ! namespace runtime
} // ! namespace rvi
