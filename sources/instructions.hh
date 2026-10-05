#ifndef RISCV_INTERPRETER_INSTRUCTIONS_HH__
#define RISCV_INTERPRETER_INSTRUCTIONS_HH__

#include <cstdint>
#include <optional>
#include <string_view>

#include "bits.hh"

namespace riscv
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

enum class ImmType
{
    R,
    I,
    S,
    B,
    U,
    J,
};

using InstructionBits = uint32_t;

class Instruction
{
public:
    Instruction( InstructionBits bits)
        : bits_( bits)
    {}

    InstructionBits bits() const { return bits_; }

public:
    InstructionBits rd()     const { return get< 7, 11>(); }
    InstructionBits funct3() const { return get<12, 14>(); }
    InstructionBits rs1()    const { return get<15, 19>(); }
    InstructionBits rs2()    const { return get<20, 24>(); }
    InstructionBits funct7() const { return get<25, 31>(); }

public:
    template<ImmType Type,
             bool SignExtend>
    InstructionBits
    imm() const
    {
        InstructionBits imm_value = 0;
        unsigned width = 0;
        if constexpr( Type == ImmType::R )
        {
            static_assert( false, "No immediate in R-Type instruction");
        } else if constexpr ( Type == ImmType::I )
        {
            imm_value = get<20, 31>();
            width = 12;
        } else if constexpr ( Type == ImmType::S )
        {
            InstructionBits imm_11_5 = get<25, 31>();
            InstructionBits imm_4_0 = get<7, 11>();
            imm_value = (imm_11_5 << 5) | imm_4_0;
            width = 12;
        } else if constexpr ( Type == ImmType::B )
        {
            InstructionBits imm_12 = get<31, 31>();
            InstructionBits imm_10_5 = get<25, 30>();
            InstructionBits imm_4_1 = get<8, 11>();
            InstructionBits imm_11 = get<7, 7>();
            imm_value = (imm_12 << 12) | (imm_10_5 << 5) | (imm_4_1 << 1) | (imm_11 << 11);
            width = 13;
        } else if constexpr ( Type == ImmType::U )
        {
            imm_value = get<12, 31>();
            width = 32;
        } else if constexpr ( Type == ImmType::J )
        {
            InstructionBits imm_20 = get<31, 31>();
            InstructionBits imm_10_1 = get<21, 30>();
            InstructionBits imm_11 = get<20, 20>();
            InstructionBits imm_19_12 = get<12, 19>();
            imm_value = (imm_20 << 20) | (imm_10_1 << 1) | (imm_11 << 11) | (imm_19_12 << 12);
            width = 21;
        } else
        {
            static_assert( false, "Unreachable");
        }

        if constexpr ( SignExtend )
        {
            return bits::signExtend<InstructionBits>( imm_value, width);
        } else
        {
            return imm_value;
        }
    }

private:
    template<unsigned First, unsigned Last>
    InstructionBits
    get() const
    {
        return bits::getField<First, Last, InstructionBits>( bits_);
    }

private:
    InstructionBits bits_;

};

} // ! namespace isa
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_INSTRUCTIONS_HH__
