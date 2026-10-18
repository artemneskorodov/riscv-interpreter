#include "interpreter/interpreter.hh"
#include "insn/dispatch.hh"
#include "isa/decoder.hh"
#include "runtime/syscalls.hh"
#include "helpers/diagnostics.hh"

namespace rvi
{
namespace interpreter
{

namespace
{

InterpreterResult<RunAction>
handleStepResult( CPUModel& cpu_model,
                  insn::InstructionEffect effect)
{
    if ( effect.needTrapHandler() )
    {
        if ( effect.trapCause() == isa::ExceptionCause::UserEcall )
        {
            InterpreterResult<RunAction> result = runtime::handleEnvironmentCall( cpu_model);
            if ( result.ok() )
            {
                cpu_model.setPC( cpu_model.getPC() + sizeof( isa::InstructionBits));
            }
            return result;
        } else
        {
            return RunAction( UnhandledTrap{
                .trap = effect.trap.value(),
                .pc = cpu_model.getPC(),
            });
        }
    }

    if ( effect.defaultAdvancePC() )
    {
        cpu_model.setPC( cpu_model.getPC() + sizeof( isa::InstructionBits));
    } else
    {
        cpu_model.setPC( effect.nextPC());
    }

    return RunAction( Continue());
}

} // ! anonymous namespace

InterpreterResult<RunAction>
runSingleInstruction( CPUModel& cpu_model)
{
    auto load_bits = cpu_model.memory().loadExecutable( cpu_model.getPC());
    if ( !load_bits.ok() )
    {
        return handleStepResult(
            cpu_model,
            insn::InstructionEffect{ .trap = load_bits.error() });
    }

    isa::InstructionBits bits = load_bits.value();
    auto encoding = isa::matchInstruction( bits);
    if ( !encoding.has_value() )
    {
        RVI_DEBUG( "Error while matching instruction: pc = {:#x}, bits = {:#x}",
                   cpu_model.getPC(),
                   bits);
        insn::InstructionEffect effect = insn::InstructionEffect{
            .trap = isa::Trap{
                .cause = isa::ExceptionCause::IllegalInstruction,
                .value = bits,
            },
        };
        return handleStepResult( cpu_model, effect);
    }

    insn::InstructionEffect effect = insn::execute( cpu_model, bits, encoding->instruction);
    return handleStepResult( cpu_model, effect);
}

} // ! namespace interpreter
} // ! namespace rvi
