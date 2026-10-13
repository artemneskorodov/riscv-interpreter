#ifndef RISCV_INTERPRETER_DISASM_HH__
#define RISCV_INTERPRETER_DISASM_HH__

#include <string>
#include <cstdint>

#include "decoder.hh"

namespace riscv
{
namespace disasm
{

std::string instrToStr( isa::InstructionBits bits, uint32_t pc);

} // ! namespace disasm
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_DISASM_HH__
