#ifndef RISCV_INTERPRETER_CPU_HH__
#define RISCV_INTERPRETER_CPU_HH__

#include <cstddef>
#include <cstdint>
#include <array>
#include <string>

namespace rv
{

using RegisterType = uint32_t;

struct CPU
{
    static constexpr std::size_t kProgrammersRegistersNumber = 32;
    static constexpr std::size_t kMemorySize = 0x2000;

    std::array<RegisterType, kProgrammersRegistersNumber> x;
    RegisterType pc;
    std::array<std::byte, kMemorySize> memory;

    bool
    valid() const
    {
        if ( x[0] != 0 )
        {
            return false;
        }
        if ( pc % sizeof( RegisterType) != 0 )
        {
            return false;
        }
        return true;
    };

    std::string
    stateString() const
    {
        std::string result = "";
        for ( std::size_t i = 0; i != kProgrammersRegistersNumber; ++i )
        {
            result += "x" + std::to_string( i) + " = " + std::to_string( x[i]) + "\n";
        }
        result += "pc = " + std::to_string( pc) + "\n";
        for ( uint32_t addr = 0x1000; addr != 0x1004; ++addr )
        {
            result += "mem[" + std::to_string( addr) + "] = " + std::to_string( static_cast<uint8_t>( memory[addr])) + "\n"; 
        }
        return result;
    }
};

} // ! namespace rv

#endif // ! RISCV_INTERPRETER_CPU_HH__
