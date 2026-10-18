#ifndef RVI_INTERPRETER_INTERPRETER_HH__
#define RVI_INTERPRETER_INTERPRETER_HH__

#include <variant>

#include "isa/cause.hh"
#include "machine/cpu.hh"
#include "helpers/error.hh"

namespace rvi
{
namespace interpreter
{

struct Continue
{};

struct Exit
{
    int status;
};

struct UnhandledTrap
{
    isa::Trap trap;
    uint32_t pc;
};

using RunAction = std::variant<Continue, Exit, UnhandledTrap>;

InterpreterResult<RunAction> runSingleInstruction( CPUModel& cpu_model);

} // ! namespace interpreter
} // ! namespace rvi

#endif // ! RVI_INTERPRETER_INTERPRETER_HH__
