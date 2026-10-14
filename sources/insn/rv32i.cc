#include "insn/execute.hh"
#include "insn/helpers.hh"
#include "bits.hh"

namespace riscv
{
namespace insn
{

Expected<StepResult>
execute_add( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) + c.getX( i.rs2()));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_addi( CPUModel& c,
              isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>());
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_and( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) & c.getX( i.rs2()));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_andi( CPUModel& c,
              isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) & i.imm<isa::ImmType::I, true>());
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_auipc( CPUModel& c,
               isa::Instruction i)
{
    c.setX( i.rd(), c.getPC() + i.imm<isa::ImmType::U, true>());
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_beq( CPUModel& c,
             isa::Instruction i)
{
    if ( c.getX( i.rs1()) == c.getX( i.rs2()) )
    {
        c.setPC( c.getPC() + i.imm<isa::ImmType::B, true>());
    } else
    {
        advancePC( c);
    }
    return StepResult::Continue;
}

Expected<StepResult>
execute_bge( CPUModel& c,
             isa::Instruction i)
{
    if ( asSigned( c.getX( i.rs1())) >= asSigned( c.getX( i.rs2())) )
    {
        c.setPC( c.getPC() + i.imm<isa::ImmType::B, true>());
    } else
    {
        advancePC( c);
    }
    return StepResult::Continue;
}

Expected<StepResult>
execute_bgeu( CPUModel& c,
              isa::Instruction i)
{
    if ( c.getX( i.rs1()) >= c.getX( i.rs2()) )
    {
        c.setPC( c.getPC() + i.imm<isa::ImmType::B, true>());
    } else
    {
        advancePC( c);
    }
    return StepResult::Continue;
}

Expected<StepResult>
execute_blt( CPUModel& c,
             isa::Instruction i)
{
    if ( asSigned( c.getX( i.rs1())) < asSigned( c.getX( i.rs2())) )
    {
        c.setPC( c.getPC() + i.imm<isa::ImmType::B, true>());
    } else
    {
        advancePC( c);
    }
    return StepResult::Continue;
}

Expected<StepResult>
execute_bltu( CPUModel& c,
              isa::Instruction i)
{
    if ( c.getX( i.rs1()) < c.getX( i.rs2()) )
    {
        c.setPC( c.getPC() + i.imm<isa::ImmType::B, true>());
    } else
    {
        advancePC( c);
    }
    return StepResult::Continue;
}

Expected<StepResult>
execute_bne( CPUModel& c,
             isa::Instruction i)
{
    if ( c.getX( i.rs1()) != c.getX( i.rs2()) )
    {
        c.setPC( c.getPC() + i.imm<isa::ImmType::B, true>());
    } else
    {
        advancePC( c);
    }
    return StepResult::Continue;
}

Expected<StepResult>
execute_ebreak( CPUModel& c,
                isa::Instruction /*i*/)
{
    advancePC( c);
    return StepResult::Breakpoint;
}

Expected<StepResult>
execute_ecall( CPUModel& c,
               isa::Instruction /*i*/)
{
    advancePC( c);
    return StepResult::EnvironmentCall;
}

Expected<StepResult>
execute_fence( CPUModel& c,
               isa::Instruction /*i*/)
{
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_jal( CPUModel& c,
             isa::Instruction i)
{
    // TODO check alignment and report and error if it is incorrect
    c.setX( i.rd(), c.getPC() + sizeof( isa::InstructionBits));
    c.setPC( c.getPC() + i.imm<isa::ImmType::J, true>());
    return StepResult::Continue;
}

Expected<StepResult>
execute_jalr( CPUModel& c,
              isa::Instruction i)
{
    // TODO check alignment and report and error if it is incorrect
    uint32_t t = c.getPC() + sizeof( isa::InstructionBits);
    uint32_t target = c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>();
    c.setPC( target & ~uint32_t( 1));
    c.setX( i.rd(), t);
    return StepResult::Continue;
}

Expected<StepResult>
execute_lb( CPUModel& c,
            isa::Instruction i)
{
    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>();
    Expected<uint8_t> value = c.memory().load<uint8_t>( addr);
    if ( !value.ok() )
    {
        return value.error();
    }

    c.setX( i.rd(), bits::signExtend<uint32_t>( value.value(), 8));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_lbu( CPUModel& c,
             isa::Instruction i)
{
    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>();
    Expected<uint8_t> value = c.memory().load<uint8_t>( addr);
    if ( !value.ok() )
    {
        return value.error();
    }

    c.setX( i.rd(), value.value());
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_lh( CPUModel& c,
            isa::Instruction i)
{
    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>();
    Expected<uint16_t> value = c.memory().load<uint16_t>( addr);
    if ( !value.ok() )
    {
        return value.error();
    }

    c.setX( i.rd(), bits::signExtend<uint32_t>( value.value(), 16));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_lhu( CPUModel& c,
             isa::Instruction i)
{
    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>();
    Expected<uint16_t> value = c.memory().load<uint16_t>( addr);
    if ( !value.ok() )
    {
        return value.error();
    }

    c.setX( i.rd(), value.value());
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_lui( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), i.imm<isa::ImmType::U, false>());
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_lw( CPUModel& c,
            isa::Instruction i)
{
    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>();
    Expected<uint32_t> value = c.memory().load<uint32_t>( addr);
    if ( !value.ok() )
    {
        return value.error();
    }

    c.setX( i.rd(), value.value());
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_or( CPUModel& c,
            isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) | c.getX( i.rs2()));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_ori( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) | i.imm<isa::ImmType::I, true>());
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_pause( CPUModel& c,
               isa::Instruction /*i*/)
{
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_sb( CPUModel& c,
            isa::Instruction i)
{
    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::S, true>();
    uint8_t value = static_cast<uint8_t>( c.getX( i.rs2()));
    Expected<void> result = c.memory().store<uint8_t>( addr, value);
    if ( !result.ok() )
    {
        return result.error();
    }

    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_sh( CPUModel& c,
            isa::Instruction i)
{
    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::S, true>();
    uint16_t value = static_cast<uint16_t>( c.getX( i.rs2()));
    auto result = c.memory().store<uint16_t>( addr, value);
    if ( !result.ok() )
    {
        return result.error();
    }

    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_sll( CPUModel& c,
             isa::Instruction i)
{
    uint32_t shift = lowerFive( c.getX( i.rs2()));
    c.setX( i.rd(), c.getX( i.rs1()) << shift);
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_slli( CPUModel& c,
              isa::Instruction i)
{
    uint32_t shift = lowerFive( i.imm<isa::ImmType::I, false>());
    c.setX( i.rd(), c.getX( i.rs1()) << shift);
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_slli_rv32( CPUModel& c,
                   isa::Instruction i)
{
    return execute_slli( c, i);
}

Expected<StepResult>
execute_slt( CPUModel& c,
             isa::Instruction i)
{
    bool r = asSigned( c.getX( i.rs1())) < asSigned( c.getX( i.rs2()));
    c.setX( i.rd(), r ? 1 : 0);
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_slti( CPUModel& c,
              isa::Instruction i)
{
    bool r = asSigned( c.getX( i.rs1())) < asSigned( i.imm<isa::ImmType::I, true>());
    c.setX( i.rd(), r ? 1 : 0);
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_sltiu( CPUModel& c,
               isa::Instruction i)
{
    bool r = c.getX( i.rs1()) < i.imm<isa::ImmType::I, true>();
    c.setX( i.rd(), r ? 1 : 0);
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_sltu( CPUModel& c,
              isa::Instruction i)
{
    bool r = c.getX( i.rs1()) < c.getX( i.rs2());
    c.setX( i.rd(), r ? 1 : 0);
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_sra( CPUModel& c,
             isa::Instruction i)
{
    uint32_t shift = lowerFive( c.getX( i.rs2()));
    c.setX( i.rd(), arithmeticShiftRight( c.getX( i.rs1()), shift));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_srai( CPUModel& c,
              isa::Instruction i)
{
    uint32_t shift = lowerFive( i.imm<isa::ImmType::I, false>());
    c.setX( i.rd(), arithmeticShiftRight( c.getX( i.rs1()), shift));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_srai_rv32( CPUModel& c,
                   isa::Instruction i)
{
    return execute_srai( c, i);
}

Expected<StepResult>
execute_srl( CPUModel& c,
             isa::Instruction i)
{
    uint32_t shift = lowerFive( c.getX( i.rs2()));
    c.setX( i.rd(), c.getX( i.rs1()) >> shift);
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_srli( CPUModel& c,
              isa::Instruction i)
{
    uint32_t shift = lowerFive( i.imm<isa::ImmType::I, false>());
    c.setX( i.rd(), c.getX( i.rs1()) >> shift);
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_srli_rv32( CPUModel& c,
                   isa::Instruction i)
{
    return execute_srli( c, i);
}

Expected<StepResult>
execute_sub( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) - c.getX( i.rs2()));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_sw( CPUModel& c,
            isa::Instruction i)
{
    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::S, true>();
    Expected<void> r = c.memory().store<uint32_t>( addr, c.getX( i.rs2()));
    if ( !r.ok() )
    {
        return r.error();
    }
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_xor( CPUModel& c,
             isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) ^ c.getX( i.rs2()));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_xori( CPUModel& c,
              isa::Instruction i)
{
    c.setX( i.rd(), c.getX( i.rs1()) ^ i.imm<isa::ImmType::I, true>());
    advancePC( c);
    return StepResult::Continue;
}

} // ! namespace insn
} // ! namespace riscv
