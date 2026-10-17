#ifndef RVI_INTERPRETER_INTERPRETER_HH__
#define RVI_INTERPRETER_INTERPRETER_HH__

#include <variant>

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

using RunAction = std::variant<Continue, Exit>;

Expected<RunAction> runSingleInstruction( CPUModel& cpu_model);

} // ! namespace interpreter
} // ! namespace rvi

#endif // ! RVI_INTERPRETER_INTERPRETER_HH__
