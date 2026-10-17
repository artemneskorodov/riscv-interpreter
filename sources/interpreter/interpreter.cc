#include <format>

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

Expected<RunAction>
handleStepResult( CPUModel& cpu_model,
                  insn::StepResult result)
{
    switch ( result )
    {
        case insn::StepResult::Continue:
            return RunAction( Continue());
        case insn::StepResult::Breakpoint:
        {
            RVI_FAILURE( "Breakpoint exceptions are not supported");
        }
        case insn::StepResult::EnvironmentCall:
            return runtime::handleEnvironmentCall( cpu_model);
        default:
        {
            RVI_UNREACHABLE();
        }
    }
}

} // ! anonymous namespace

Expected<RunAction>
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
        return Error( ErrorCode::InvalidInstruction,
                      std::format( "Unknown instruction: {}", bits));
    }

    auto execute_result = insn::execute( cpu_model, bits, encoding->instruction);
    if ( !execute_result.ok() )
    {
        RVI_DEBUG( "Error while running instruction: pc = {:#x}, bits = {:#x}, "
                   "instruction = {}",
                   cpu_model.getPC(),
                   bits,
                   encoding->name);
        return execute_result.error();
    }

    return handleStepResult( cpu_model, execute_result.value());
}

} // ! namespace interpreter
} // ! namespace rvi
