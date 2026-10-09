#include <cstring>
#include <iostream>

#include "instructions.hh"
#include "decoder.hh"
#include "rv32i_behaviour.generated.hh"

namespace riscv
{
namespace insn
{

void
runSingleInstruction( CPUModel& cpu_model)
{
    isa::InstructionBits bits = 0;
    auto load_bits = cpu_model.memory().loadExecutable( cpu_model.getPC());
    if ( !load_bits.ok() )
    {
        std::cout << "Error while load instruction: " << load_bits.error().message << std::endl;
        return ;
    }

    auto encoding = isa::matchInstruction( bits);
    if ( !encoding )
    {
        std::cout << "Unknown instruction: " << std::hex << bits << std::endl;
        return ;
    }

    if ( !runGeneratedInstruction( cpu_model, encoding->instruction, bits) )
    {
        std::cout << "Instruction behaviour is not implemented: "
                  << static_cast<int>( encoding->instruction) << std::endl;
    }
}

} // ! namespace insn
} // ! namespace riscv
