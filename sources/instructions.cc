#include <cstring>
#include <iostream>

#include "instructions.hh"
#include "decoder.hh"
#include "rv32i_behaviour.generated.hh"

namespace riscv
{
namespace insn
{

Expected<void>
runSingleInstruction( CPUModel& cpu_model)
{
    auto load_bits = cpu_model.memory().loadExecutable( cpu_model.getPC());
    if ( !load_bits.ok() )
    {
        return load_bits.error();
    }

    isa::InstructionBits bits = load_bits.value();

    auto encoding = isa::matchInstruction( bits);
    if ( !encoding.has_value() )
    {
        return Error( ErrorCode::UnknownInstruction,
                      std::format( "Unknown instruction: {}", bits));
    }

    auto r = runGeneratedInstruction( cpu_model, encoding->instruction, bits);
    if ( !r.ok() )
    {
        return r.error();
    }
    return Expected<void>();
}

} // ! namespace insn
} // ! namespace riscv
