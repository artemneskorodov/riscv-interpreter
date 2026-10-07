#include <cstdint>
#include <iostream>

#include "cpu_model.hh"
#include "instructions.hh"

namespace
{

constexpr uint32_t
encodeAdd( unsigned rd,
           unsigned rs1,
           unsigned rs2)
{
    return (rs2 << 20) | (rs1 << 15) | (rd << 7) | 0x33u;
}

constexpr uint32_t
encodeAddi( unsigned rd,
            unsigned rs1,
            uint32_t immediate)
{
    return ((immediate & 0xfffu) << 20) | (rs1 << 15) | (rd << 7) | 0x13u;
}

bool
check( bool condition,
       const char* message)
{
    if ( !condition )
    {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

} // ! anonymous namespace

int
main()
{
    riscv::CPUModel cpu_model;
    cpu_model.setX( 1, 10);
    cpu_model.setX( 2, 32);

    uint32_t instruction = encodeAdd( 3, 1, 2);
    cpu_model.store( 0, sizeof( instruction), &instruction);
    riscv::insn::runSingleInstruction( cpu_model);

    bool success = true;
    success &= check( cpu_model.getX( 3) == 42, "ADD result is incorrect");
    success &= check( cpu_model.getPC() == 4, "ADD did not advance PC");

    instruction = encodeAddi( 4, 3, 0xfffu); // -1 as a 12-bit immediate
    cpu_model.store( 4, sizeof( instruction), &instruction);
    riscv::insn::runSingleInstruction( cpu_model);

    success &= check( cpu_model.getX( 4) == 41, "ADDI sign extension is incorrect");
    success &= check( cpu_model.getPC() == 8, "ADDI did not advance PC");

    instruction = encodeAdd( 0, 1, 2);
    cpu_model.store( 8, sizeof( instruction), &instruction);
    riscv::insn::runSingleInstruction( cpu_model);

    success &= check( cpu_model.getX( 0) == 0, "generated write changed x0");
    success &= check( cpu_model.getPC() == 12, "second ADD did not advance PC");
    return success ? 0 : 1;
}
