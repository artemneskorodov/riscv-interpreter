#ifndef RVI_TOOLS_DISASSEMBLER_HH__
#define RVI_TOOLS_DISASSEMBLER_HH__

#include <string>
#include <cstdint>

#include "isa/decoder.hh"

namespace rvi
{
namespace tools
{

std::string instructionToString( isa::InstructionBits bits, uint32_t pc);

} // ! namespace tools
} // ! namespace rvi

#endif // ! RVI_TOOLS_DISASSEMBLER_HH__
