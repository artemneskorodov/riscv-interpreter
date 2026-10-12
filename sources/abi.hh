#ifndef RISCV_INTERPRETER_ABI_HH__
#define RISCV_INTERPRETER_ABI_HH__

#include <cstdint>
#include <vector>
#include <string_view>

#include "errors.hh"
#include "memory.hh"
#include "cpu_model.hh"

namespace riscv
{
namespace abi
{

//
// Guest stack parameters.
//
constexpr uint32_t kDefaultStackSize = 1 * 1024 * 1024;
constexpr uint32_t kDefaultStackTop = 0xc0000000;

//
// Allocates guest stack in memory.
//
Expected<void> allocateGuestStack( mem::Memory& memory,
                                   uint32_t top = kDefaultStackTop,
                                   uint32_t size = kDefaultStackSize);

//
// Store program arguments to guest stack, creates main parameters and points sp.
//
Expected<void> prepareMainArguments( CPUModel& cpu_model,
                                     const std::vector<std::string_view>& args,
                                     uint32_t initial_sp = kDefaultStackTop);

//
// RISC-V calling convention register usage.
//
class AbiRegisterName
{
public:
    enum RegName
    {
        // Hard-wired zero
        zero = 0,

        // Return address
        ra = 1,

        // Stack pointer
        sp = 2,

        // Global pointer
        gp = 3,

        // Thread pointer
        tp = 4,

        // Temporaries
        t0 = 5,
        t1 = 6,
        t2 = 7,

        // Saved register
        s0 = 8,

        // Frame pointer
        fp = 8,

        // Saved Register
        s1 = 9,

        // Function arguments or return values
        a0 = 10,
        a1 = 11,

        // Function arguments 
        a2 = 12,
        a3 = 13,
        a4 = 14,
        a5 = 15,
        a6 = 16,
        a7 = 17,

        // Saved registers
        s2 = 18,
        s3 = 19,
        s4 = 20,
        s5 = 21,
        s6 = 22,
        s7 = 23,
        s8 = 24,
        s9 = 25,
        s10 = 26,
        s11 = 27,

        // Temporaries
        t3 = 28,
        t4 = 29,
        t5 = 30,
        t6 = 31,
    };

public:
    AbiRegisterName( unsigned n)
        : reg_( static_cast<RegName>( n))
    {}

    AbiRegisterName( RegName reg)
        : reg_( reg)
    {}

    operator unsigned() const
    {
        return static_cast<unsigned>( reg_);
    }

private:
    RegName reg_;

};

} // ! namespace abi
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_ABI_HH__
