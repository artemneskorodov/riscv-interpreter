#include <cstddef>
#include <format>

#include "disasm.hh"

extern "C"
{
#include "riscv-disas.h"
}

namespace riscv
{
namespace disasm
{

std::string
instrToStr( isa::InstructionBits bits,
            uint32_t pc)
{
    constexpr std::size_t kMaxTextSize = 128;
    char instr_text[kMaxTextSize] = {};

    disasm_inst( instr_text, kMaxTextSize, rv32, pc, bits);
    return std::format( "pc={:#010x}:    {}", pc, std::string( instr_text));
}

} // ! namespace disasm
} // ! namespace riscv
