#include <iostream>

#include "cpu.hh"
#include "instructions.hh"

constexpr uint32_t program[] = {
    0x00001537, // lui   x10, 0x1
    0x00500093, // addi  x1, x0, 5
    0x00700113, // addi  x2, x0, 7
    0x002081b3, // add   x3, x1, x2
    0x40118233, // sub   x4, x3, x1
    0x00452023, // sw    x4, 0(x10)
    0x00052283, // lw    x5, 0(x10)
    0x00228463, // beq   x5, x2, +8
    0x06300313, // addi  x6, x0, 99
    0x008003ef, // jal   x7, +8
    0x05800413, // addi  x8, x0, 88
    0x03800493, // addi  x9, x0, 0x38
    0x00048067, // jalr  x0, 0(x9)
    0x04d00593, // addi  x11, x0, 77
    0x02a00613, // addi  x12, x0, 42
};

int
main( void)
{
    rv::CPU cpu_model{};

    std::memcpy( cpu_model.memory.data(), program, sizeof( program));
    cpu_model.pc = 0;

    while ( cpu_model.pc < sizeof( program) )
    {
        rv::instr::runInstruction( cpu_model);
    }

    std::cout << cpu_model.stateString();
}
