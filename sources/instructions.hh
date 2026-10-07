#ifndef RISCV_INTERPRETER_INSTRUCTIONS_HH__
#define RISCV_INTERPRETER_INSTRUCTIONS_HH__

#include "cpu_model.hh"

namespace riscv
{
namespace insn
{

void runSingleInstruction( CPUModel& cpu_model);

} // ! namespace insn
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_INSTRUCTIONS_HH__
