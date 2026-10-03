#ifndef RISCV_INTERPRETER_INSTRUCTIONS_HH__
#define RISCV_INTERPRETER_INSTRUCTIONS_HH__

#include <cstdint>
#include <optional>
#include <string_view>

namespace riscv
{
namespace isa
{

enum class Instruction
{
    #define DECLARE_INSN(name_, match_, mask_) instr_ ## name_,
    #include "riscv-opcodes/encoding.out.h"
    #undef DECLARE_INSN
};

struct InstructionEncoding
{
    Instruction instruction;
    uint32_t match;
    uint32_t mask;
    std::string_view name;
};

constexpr InstructionEncoding kEncodings[] = {
    #define DECLARE_INSN(name_, match_, mask_) \
        InstructionEncoding{ Instruction::instr_ ## name_, match_, mask_, #name_},
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
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_INSTRUCTIONS_HH__
