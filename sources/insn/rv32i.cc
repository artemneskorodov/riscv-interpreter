#include "insn/dispatch.hh"
#include "insn/helpers.hh"
#include "helpers/bits.hh"

namespace rvi
{
namespace insn
{

//
// Arithmetic instructions
//

InstructionEffect
execute_add( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) + c.getX( i.rs2()));
    return {};
}

InstructionEffect
execute_addi( CPUModel& c,
              isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>());
    return {};
}

InstructionEffect
execute_and( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) & c.getX( i.rs2()));
    return {};
}

InstructionEffect
execute_andi( CPUModel& c,
              isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) & i.imm<isa::ImmType::I, true>());
    return {};
}

InstructionEffect
execute_auipc( CPUModel& c,
               isa::Instruction i)
{
    c.setX( i.rd(), c.getPC() + i.imm<isa::ImmType::U, true>());
    return {};
}

InstructionEffect
execute_sub( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) - c.getX( i.rs2()));
    return {};
}

InstructionEffect
execute_or( CPUModel& c,
            isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) | c.getX( i.rs2()));
    return {};
}

InstructionEffect
execute_ori( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) | i.imm<isa::ImmType::I, true>());
    return {};
}

InstructionEffect
execute_xor( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) ^ c.getX( i.rs2()));
    return {};
}

InstructionEffect
execute_xori( CPUModel& c,
              isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) ^ i.imm<isa::ImmType::I, true>());
    return {};
}

//
// Branch compare instructions
//

namespace
{

InstructionEffect
executeCompareBranchInsn( CPUModel& c,
                          isa::Instruction i,
                          bool cmp_result)
{
    if ( cmp_result )
    {
        return InstructionEffect{
            .next_pc = c.getPC() + i.imm<isa::ImmType::B, true>(),
        };
    } else
    {
        return {};
    }
}

} // ! anonymous namespace

InstructionEffect
execute_beq( CPUModel& c,
             isa::Instruction i)
{
    bool r = c.getX( i.rs1()) == c.getX( i.rs2());
    return executeCompareBranchInsn( c, i, r);
}

InstructionEffect
execute_bge( CPUModel& c,
             isa::Instruction i)
{
    bool r = asSigned( c.getX( i.rs1())) >= asSigned( c.getX( i.rs2()));
    return executeCompareBranchInsn( c, i, r);
}

InstructionEffect
execute_bgeu( CPUModel& c,
              isa::Instruction i)
{
    bool r = c.getX( i.rs1()) >= c.getX( i.rs2());
    return executeCompareBranchInsn( c, i, r);
}

InstructionEffect
execute_blt( CPUModel& c,
             isa::Instruction i)
{
    bool r = asSigned( c.getX( i.rs1())) < asSigned( c.getX( i.rs2()));
    return executeCompareBranchInsn( c, i, r);
}

InstructionEffect
execute_bltu( CPUModel& c,
              isa::Instruction i)
{
    bool r = c.getX( i.rs1()) < c.getX( i.rs2());
    return executeCompareBranchInsn( c, i, r);
}

InstructionEffect
execute_bne( CPUModel& c,
             isa::Instruction i)
{
    bool r = c.getX( i.rs1()) != c.getX( i.rs2());
    return executeCompareBranchInsn( c, i, r);
}

//
// Traps
//

InstructionEffect
execute_ebreak( CPUModel& /*c*/,
                isa::Instruction /*i*/)
{
    return InstructionEffect{
        .trap = isa::Trap{
            .cause = isa::ExceptionCause::Breakpoint,
        },
    };
}

InstructionEffect
execute_ecall( CPUModel& /*c*/,
               isa::Instruction /*i*/)
{
    return InstructionEffect{
        .trap = isa::Trap{
            .cause = isa::ExceptionCause::UserEcall,
        },
    };
}

//
// In interpreter fence and pause instructions do not do anything.
//

InstructionEffect
execute_fence( CPUModel& /*c*/,
               isa::Instruction /*i*/)
{
    return {};
}

InstructionEffect
execute_pause( CPUModel& /*c*/,
               isa::Instruction /*i*/)
{
    return {};
}

//
// Jumps
//

InstructionEffect
execute_jal( CPUModel& c,
             isa::Instruction i)
{
    // TODO check alignment and report an error if it is incorrect
    c.setX( i.rd(), c.getPC() + sizeof( isa::InstructionBits));
    return InstructionEffect{
        .next_pc = c.getPC() + i.imm<isa::ImmType::J, true>(),
    };
}

InstructionEffect
execute_jalr( CPUModel& c,
              isa::Instruction i)
{
    // TODO check alignment and report and error if it is incorrect
    uint32_t t = c.getPC() + sizeof( isa::InstructionBits);
    uint32_t target = c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>();
    c.setX( i.rd(), t);
    return InstructionEffect{
        .next_pc = target & ~uint32_t( 1),
    };
}

//
// Loads
//

namespace
{

template<typename T, bool SignExtend>
InstructionEffect
executeLoad( CPUModel& c,
             isa::Instruction i)
{
    static_assert( std::is_same_v<T, uint8_t>
                   || std::is_same_v<T, uint16_t>
                   || std::is_same_v<T, uint32_t>);

    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>();
    machine::MemoryResult<T> value = c.memory().load<T>( addr);
    if ( !value.ok() )
    {
        return InstructionEffect{ .trap = value.error() };
    }

    uint32_t result = SignExtend
                      ? bits::signExtend<uint32_t>( value.value(), 8 * sizeof( T))
                      : static_cast<uint32_t>( value.value());
    c.setX( i.rd(), result);
    return {};
}

} // ! anonymous namespace

InstructionEffect
execute_lb( CPUModel& c,
            isa::Instruction i)
{
    return executeLoad<uint8_t, true>( c, i);
}

InstructionEffect
execute_lbu( CPUModel& c,
             isa::Instruction i)
{
    return executeLoad<uint8_t, false>( c, i);
}

InstructionEffect
execute_lh( CPUModel& c,
            isa::Instruction i)
{
    return executeLoad<uint16_t, true>( c, i);
}

InstructionEffect
execute_lhu( CPUModel& c,
             isa::Instruction i)
{
    return executeLoad<uint16_t, false>( c, i);
}

InstructionEffect
execute_lw( CPUModel& c,
            isa::Instruction i)
{
    return executeLoad<uint32_t, false>( c, i);
}

InstructionEffect
execute_lui( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), i.imm<isa::ImmType::U, false>());
    return {};
}

//
// Stores
//

namespace
{

template<typename T>
InstructionEffect
executeStore( CPUModel& c,
              isa::Instruction i)
{
    static_assert( std::is_same_v<T, uint8_t>
                   || std::is_same_v<T, uint16_t>
                   || std::is_same_v<T, uint32_t>);

    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::S, true>();
    T value = static_cast<T>( c.getX( i.rs2()));

    machine::MemoryResult<void> r = c.memory().store<T>( addr, value);
    if ( !r.ok() )
    {
        return InstructionEffect{ .trap = r.error() };
    }

    return {};
}

} // ! anonymous namespace

InstructionEffect
execute_sb( CPUModel& c,
            isa::Instruction i)
{
    return executeStore<uint8_t>( c, i);
}

InstructionEffect
execute_sh( CPUModel& c,
            isa::Instruction i)
{
    return executeStore<uint16_t>( c, i);
}

InstructionEffect
execute_sw( CPUModel& c,
            isa::Instruction i)
{
    return executeStore<uint32_t>( c, i);
}

//
// Compare instructions
//

namespace
{

InstructionEffect
executeCompareInsn( CPUModel& c,
                           isa::Instruction i,
                           bool r)
{
    c.setX( i.rd(), r ? 1 : 0);
    return {};
}

} // ! anonymous namespace

InstructionEffect
execute_slt( CPUModel& c,
             isa::Instruction i)
{
    bool r = asSigned( c.getX( i.rs1())) < asSigned( c.getX( i.rs2()));
    return executeCompareInsn( c, i, r);
}

InstructionEffect
execute_slti( CPUModel& c,
              isa::Instruction i)
{
    bool r = asSigned( c.getX( i.rs1())) < asSigned( i.imm<isa::ImmType::I, true>());
    return executeCompareInsn( c, i, r);
}

InstructionEffect
execute_sltiu( CPUModel& c,
               isa::Instruction i)
{
    bool r = c.getX( i.rs1()) < i.imm<isa::ImmType::I, true>();
    return executeCompareInsn( c, i, r);
}

InstructionEffect
execute_sltu( CPUModel& c,
              isa::Instruction i)
{
    bool r = c.getX( i.rs1()) < c.getX( i.rs2());
    return executeCompareInsn( c, i, r);
}

//
// Left shifts
//

InstructionEffect
execute_sll( CPUModel& c,
             isa::Instruction i)
{
    uint32_t shift = lowerFive( c.getX( i.rs2()));
    c.setX( i.rd(), c.getX( i.rs1()) << shift);
    return {};
}

InstructionEffect
execute_slli( CPUModel& c,
              isa::Instruction i)
{
    uint32_t shift = lowerFive( i.imm<isa::ImmType::I, false>());
    c.setX( i.rd(), c.getX( i.rs1()) << shift);
    return {};
}

InstructionEffect
execute_slli_rv32( CPUModel& c,
                   isa::Instruction i)
{
    return execute_slli( c, i);
}

//
// Arithmetic right shifts
//

namespace
{

uint32_t
arithmeticShiftRight( uint32_t value,
                      uint32_t shift)
{
    RVI_ASSERT( shift == lowerFive( shift));

    if ( shift == 0 )
    {
        return value;
    }

    uint32_t result = value >> shift;
    if ( value & (uint32_t( 1) << 31) )
    {
        // Saving last bit.
        result |= std::numeric_limits<uint32_t>::max() << (32u - shift);
    }
    return result;
}

} // ! anonymous namespace

InstructionEffect
execute_sra( CPUModel& c,
             isa::Instruction i)
{
    uint32_t shift = lowerFive( c.getX( i.rs2()));
    c.setX( i.rd(), arithmeticShiftRight( c.getX( i.rs1()), shift));
    return {};
}

InstructionEffect
execute_srai( CPUModel& c,
              isa::Instruction i)
{
    uint32_t shift = lowerFive( i.imm<isa::ImmType::I, false>());
    c.setX( i.rd(), arithmeticShiftRight( c.getX( i.rs1()), shift));
    return {};
}

InstructionEffect
execute_srai_rv32( CPUModel& c,
                   isa::Instruction i)
{
    return execute_srai( c, i);
}

//
// Logical right shifts
//

InstructionEffect
execute_srl( CPUModel& c,
             isa::Instruction i)
{
    uint32_t shift = lowerFive( c.getX( i.rs2()));
    c.setX( i.rd(), c.getX( i.rs1()) >> shift);
    return {};
}

InstructionEffect
execute_srli( CPUModel& c,
              isa::Instruction i)
{
    uint32_t shift = lowerFive( i.imm<isa::ImmType::I, false>());
    c.setX( i.rd(), c.getX( i.rs1()) >> shift);
    return {};
}

InstructionEffect
execute_srli_rv32( CPUModel& c,
                   isa::Instruction i)
{
    return execute_srli( c, i);
}

} // ! namespace insn
} // ! namespace rvi
