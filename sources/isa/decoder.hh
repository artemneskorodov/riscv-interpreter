#ifndef RVI_ISA_DECODER_HH__
#define RVI_ISA_DECODER_HH__

#include <cstdint>
#include <optional>
#include <string_view>

#include "isa/instruction.hh"

namespace rvi
{
namespace isa
{

enum class Operation
{
    #define DECLARE_INSN(name_, match_, mask_) instr_ ## name_,
    #include "riscv-opcodes/encoding.out.h"
    #undef DECLARE_INSN
};

struct InstructionEncoding
{
    Operation instruction;
    uint32_t match;
    uint32_t mask;
    std::string_view name;
};

constexpr InstructionEncoding kEncodings[] = {
    #define DECLARE_INSN(name_, match_, mask_) { Operation::instr_ ## name_, match_, mask_, #name_},
    #include "riscv-opcodes/encoding.out.h"
    #undef DECLARE_INSN
};

inline constexpr std::optional<InstructionEncoding>
matchInstruction( uint32_t instr)
{
    for ( const InstructionEncoding& encoding : kEncodings )
    {
        if ( (instr & encoding.mask) == encoding.match )
        {
            return encoding;
        }
    }
    return std::nullopt;
}

} // ! namespace isa
} // ! namespace rvi

#endif // ! RVI_ISA_DECODER_HH__
