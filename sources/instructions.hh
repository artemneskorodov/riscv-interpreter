#ifndef RISCV_INTERPRETER_INSTRUCTIONS_HH__
#define RISCV_INTERPRETER_INSTRUCTIONS_HH__

#include <variant>

#include "cpu_model.hh"
#include "errors.hh"

namespace riscv
{
namespace insn
{

struct Continue
{};

struct Exit
{
    int status;
};

using RunAction = std::variant<Continue, Exit>;

Expected<RunAction> runSingleInstruction( CPUModel& cpu_model);

} // ! namespace insn
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_INSTRUCTIONS_HH__
