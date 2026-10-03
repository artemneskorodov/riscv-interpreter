#ifndef RISCV_INTERPRETER_INSTRUCTIONS_HH__
#define RISCV_INTERPRETER_INSTRUCTIONS_HH__

#include "cpu.hh"

namespace rv
{
namespace instr
{

void runInstruction( CPU& cpu);

} // ! namespace instr
} // ! namespace rv

#endif // ! RISCV_INTERPRETER_INSTRUCTIONS_HH__
