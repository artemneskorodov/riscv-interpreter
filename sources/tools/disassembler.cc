#include <cstddef>
#include <format>

#include "tools/disassembler.hh"

extern "C"
{
#include "riscv-disas.h"
}

namespace rvi
{
namespace tools
{

std::string
instructionToString( isa::InstructionBits bits,
                     uint32_t pc)
{
    constexpr std::size_t kMaxTextSize = 128;
    char instr_text[kMaxTextSize] = {};

    disasm_inst( instr_text, kMaxTextSize, rv32, pc, bits);
    return std::format( "pc={:#010x}:    {}", pc, std::string( instr_text));
}

} // ! namespace tools
} // ! namespace rvi
