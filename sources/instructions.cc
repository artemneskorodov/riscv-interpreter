#include <cstring>
#include <string_view>
#include <optional>
#include <iostream>

#include "instructions.hh"

namespace rv
{
namespace instr
{

enum class Instruction
{
    #define DECLARE_INSN( name_, match_, mask_) instr_ ## name_,
    #include "riscv-opcodes/encoding.out.h"
    #undef DECLARE_INSN
};

struct InstructionEncoding
{
    Instruction instr;
    RegisterType match;
    RegisterType mask;
    std::string_view name;
};

constexpr InstructionEncoding kInstructionEncodings[] = {
    #define DECLARE_INSN( name_, match_, mask_) { Instruction::instr_ ## name_, match_, mask_, #name_},
    #include "riscv-opcodes/encoding.out.h"
    #undef DECLARE_INSN
};

inline constexpr std::optional<InstructionEncoding>
matchInstruction( uint32_t instr)
{
    for ( const InstructionEncoding& encoding : kInstructionEncodings )
    {
        if ( (instr & encoding.mask) == encoding.match )
        {
            return encoding;
        }
    }
    return std::nullopt;
}

uint32_t
getField( uint32_t value,
          uint32_t low,
          uint32_t high)
{
    uint32_t width = high + 1 - low;
    if ( width == 32 )
    {
        return (value >> low);
    }
    return (value >> low) & ((1u << width) - 1u);
}

void
runInstructionLui( uint32_t instruction, CPU& cpu)
{
    uint32_t rd = getField( instruction, 7, 11);
    uint32_t imm = getField( instruction, 12, 31);

    if ( rd != 0 )
    {
        cpu.x[rd] = (imm << 12);
    }
    cpu.pc += 4;
}

void
runInstructionAddi( uint32_t instruction, CPU& cpu)
{
    uint32_t rd = getField( instruction, 7, 11);
    uint32_t rs1 = getField( instruction, 15, 19);
    uint32_t imm = getField( instruction, 20, 31);
    if ( rd != 0 )
    {
        cpu.x[rd] = cpu.x[rs1] + imm;
    }
    cpu.pc += 4;
}

void
runInstructionAdd( uint32_t instruction, CPU& cpu)
{
    uint32_t rd = getField( instruction, 7, 11);
    uint32_t rs1 = getField( instruction, 15, 19);
    uint32_t rs2 = getField( instruction, 20, 24);

    if ( rd != 0 )
    {
        cpu.x[rd] = cpu.x[rs1] + cpu.x[rs2];
    }
    cpu.pc += 4;
}

void
runInstructionSub( uint32_t instruction, CPU& cpu)
{
    uint32_t rd = getField( instruction, 7, 11);
    uint32_t rs1 = getField( instruction, 15, 19);
    uint32_t rs2 = getField( instruction, 20, 24);

    if ( rd != 0 )
    {
        cpu.x[rd] = cpu.x[rs1] - cpu.x[rs2];
    }
    cpu.pc += 4;
}

void
runInstructionSw( uint32_t instruction, CPU& cpu)
{
    uint32_t offset_4_0 = getField( instruction, 7, 11);
    uint32_t offset_11_5 = getField( instruction, 25, 31);
    uint32_t rs1 = getField( instruction, 15, 19);
    uint32_t rs2 = getField( instruction, 20, 24);

    uint32_t offset = (offset_11_5 << 5) | offset_4_0;
    uint32_t addr = cpu.x[rs1] + offset;
    std::memcpy( cpu.memory.data() + addr, &cpu.x[rs2], sizeof( uint32_t));
    cpu.pc += 4;

}

void
runInstructionLw( uint32_t instruction, CPU& cpu)
{
    uint32_t rd = getField( instruction, 7, 11);
    uint32_t offset = getField( instruction, 20, 31);
    uint32_t rs1 = getField( instruction, 15, 19);

    uint32_t addr = cpu.x[rs1] + offset;
    if ( rd != 0 )
    {
        std::memcpy( &cpu.x[rd], cpu.memory.data() + addr, sizeof( uint32_t));
    }
    cpu.pc += 4;
}

void
runInstructionBeq( uint32_t instruction, CPU& cpu)
{
    uint32_t rs1 = getField( instruction, 15, 19);
    uint32_t rs2 = getField( instruction, 20, 24);

    uint32_t offset_12 = getField( instruction, 31, 31);
    uint32_t offset_10_5 = getField( instruction, 25, 30);
    uint32_t offset_11 = getField( instruction, 7, 7);
    uint32_t offset_4_1 = getField( instruction, 8, 11);
    uint32_t offset = (offset_4_1 << 1) | (offset_11 << 11) | (offset_12 << 12) | (offset_10_5 << 5);
    if ( cpu.x[rs1] == cpu.x[rs2] )
    {
        cpu.pc += offset;
    } else
    {
        cpu.pc += 4;
    }
}

void
runInstructionJal( uint32_t instruction, CPU& cpu)
{
    uint32_t rd = getField( instruction, 7, 11);

    uint32_t offset_19_12 = getField( instruction, 12, 19);
    uint32_t offset_11 = getField( instruction, 20, 20);
    uint32_t offset_10_1 = getField( instruction, 21, 30);
    uint32_t offset_20 = getField( instruction, 31, 31);

    uint32_t offset = (offset_19_12 << 12) | (offset_11 << 11) | (offset_10_1 << 1) | (offset_20 << 20);
    
    if ( rd != 0 )
    {
        cpu.x[rd] = cpu.pc + 4;
    }
    cpu.pc += offset;
}

void
runInstructionJalr( uint32_t instruction, CPU& cpu)
{
    uint32_t rd = getField( instruction, 7, 11);
    uint32_t rs1 = getField( instruction, 15, 19);
    uint32_t offset = getField( instruction, 20, 31);

    uint32_t t = cpu.pc + 4;
    cpu.pc = (cpu.x[rs1] + offset) & ~1u;
    if ( rd != 0 )
    {
        cpu.x[rd] = t;
    }
}

void
runInstruction( CPU& cpu)
{
    RegisterType instruction = 0;
    std::memcpy( &instruction, cpu.memory.data() + cpu.pc, sizeof( instruction));

    auto instr_encoding = matchInstruction( instruction);
    if ( !instr_encoding )
    {
        ::exit( 1);
    }

    std::cout << instr_encoding->name << std::endl;

    if ( instr_encoding->instr == Instruction::instr_lui )
    {
        runInstructionLui( instruction, cpu);
    } else if ( instr_encoding->instr == Instruction::instr_addi )
    {
        runInstructionAddi( instruction, cpu);
    } else if ( instr_encoding->instr == Instruction::instr_add )
    {
        runInstructionAdd( instruction, cpu);
    } else if ( instr_encoding->instr == Instruction::instr_sub )
    {
        runInstructionSub( instruction, cpu);
    } else if ( instr_encoding->instr == Instruction::instr_sw )
    {
        runInstructionSw( instruction, cpu);
    } else if ( instr_encoding->instr == Instruction::instr_lw )
    {
        runInstructionLw( instruction, cpu);
    } else if ( instr_encoding->instr == Instruction::instr_beq )
    {
        runInstructionBeq( instruction, cpu);
    } else if ( instr_encoding->instr == Instruction::instr_jal )
    {
        runInstructionJal( instruction, cpu);
    } else if ( instr_encoding->instr == Instruction::instr_jalr )
    {
        runInstructionJalr( instruction, cpu);
    } else
    {
        std::cout << "Unknown instruction" << std::endl;
        ::exit( 1);
    }
}

} // ! namespace instr
} // ! namespace rv
