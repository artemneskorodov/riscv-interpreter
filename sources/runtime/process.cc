#include <cstddef>

#include "runtime/process.hh"
#include "isa/registers.hh"
#include "helpers/diagnostics.hh"

namespace rvi
{
namespace runtime
{

InterpreterResult<void>
allocateGuestStack( machine::Memory& memory,
                    uint32_t top,
                    uint32_t size)
{
    auto r = memory.allocate( top - size,
                              size,
                              machine::MemoryPermissions::READ | machine::MemoryPermissions::WRITE,
                              nullptr, 0);
    if ( !r.ok() )
    {
        return r.error();
    }
    return InterpreterResult<void>();
}

InterpreterResult<void>
prepareMainArguments( CPUModel& cpu_model,
                      const std::vector<std::string_view>& args,
                      uint32_t initial_sp)
{
    std::vector<uint32_t> pointers;
    pointers.reserve( args.size());

    uint32_t sp = initial_sp;
    uint8_t zero = 0;
    for ( const std::string_view& arg : args )
    {
        sp -= (arg.length() + 1);
        cpu_model.memory().storeTrusted( sp, arg.length(), arg.data());
        cpu_model.memory().storeTrusted( sp + arg.length(), 1, &zero);
        RVI_DEBUG( "Argument {} is stored in [{:#x}, {:#x})", arg, sp, sp + arg.length());
        pointers.emplace_back( sp);
    }

    //
    // Loading argc and argv[] to guest stack together.
    // The resulting sp must be 16-byte aligned as RISCV expects.
    //
    uint32_t pointers_size = sizeof( uint32_t) * pointers.size();
    uint32_t argc = args.size();

    sp -= (pointers_size + sizeof( argc));
    sp &= ~0xf;
    cpu_model.memory().storeTrusted( sp, sizeof( argc), &argc);
    cpu_model.memory().storeTrusted( sp + sizeof( argc), pointers_size, pointers.data());

    //
    // Setting stack pointer
    //
    RVI_DEBUG( "sp = {:#x}", sp);
    cpu_model.setX( isa::RegisterName::sp, sp);

    return InterpreterResult<void>();
}

} // ! namespace runtime
} // ! namespace rvi
