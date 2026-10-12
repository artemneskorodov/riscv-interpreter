#ifndef RISCV_INTERPRETER_INSTRUCTIONS_HH__
#define RISCV_INTERPRETER_INSTRUCTIONS_HH__

#include "cpu_model.hh"
#include "errors.hh"

namespace riscv
{
namespace insn
{

Expected<void> runSingleInstruction( CPUModel& cpu_model);

} // ! namespace insn
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_INSTRUCTIONS_HH__
