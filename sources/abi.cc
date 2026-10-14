#include <cstddef>

#include "abi.hh"
#include "debug.hh"

namespace riscv
{
namespace abi
{

Expected<void>
allocateGuestStack( mem::Memory& memory,
                    uint32_t top,
                    uint32_t size)
{
    auto r = memory.allocate( top - size,
                              size,
                              mem::MemoryPermissions::READ | mem::MemoryPermissions::WRITE,
                              nullptr, 0);
    if ( !r.ok() )
    {
        return r.error();
    }
    return Expected<void>();
}

Expected<void>
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
    // Loading argv[] to guest stack
    //
    uint32_t pointers_size = sizeof( uint32_t) * pointers.size();
    sp -= pointers_size;
    cpu_model.memory().storeTrusted( sp, pointers_size, pointers.data());

    //
    // Loading argc to guest stack
    //
    uint32_t count = args.size();
    sp -= sizeof( count);
    cpu_model.memory().storeTrusted( sp, sizeof( count), &count);

    //
    // Setting stack pointer
    //
    RVI_DEBUG( "sp = {:#x}", sp);
    cpu_model.setX( AbiRegisterName::sp, sp);

    return Expected<void>();
}

} // ! namespace abi
} // ! namespace riscv
