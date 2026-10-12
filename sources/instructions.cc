#include <cstring>
#include <iostream>

#include "instructions.hh"
#include "decoder.hh"
#include "rv32i_behaviour.generated.hh"
#include "debug.hh"

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
    return Expected<void>();
}

} // ! namespace insn
} // ! namespace riscv
