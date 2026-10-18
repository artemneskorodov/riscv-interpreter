#ifndef RVI_INSN_DISPATCH_HH__
#define RVI_INSN_DISPATCH_HH__

#include <optional>

#include "helpers/error.hh"
#include "machine/cpu.hh"
#include "isa/instruction.hh"
#include "isa/cause.hh"
#include "isa/decoder.hh"
#include "helpers/diagnostics.hh"

namespace rvi
{
namespace insn
{

struct InstructionEffect
{
    bool
    needTrapHandler() const
    {
        return trap.has_value();
    }

    isa::ExceptionCause
    trapCause() const
    {
        RVI_ASSERT( needTrapHandler());
        return trap.value().cause;
    }

    bool
    defaultAdvancePC() const
    {
        return !next_pc.has_value();
    }

    uint32_t
    nextPC() const
    {
        RVI_ASSERT( !defaultAdvancePC());
        return next_pc.value();
    }

    std::optional<isa::Trap> trap = std::nullopt;
    std::optional<uint32_t> next_pc = std::nullopt;
};

#define DECLARE_INSN( name_, match_, mask_) \
    InstructionEffect execute_ ## name_( CPUModel& cpu, isa::Instruction insn);

#include "riscv-opcodes/encoding.out.h"

#undef DECLARE_INSN

inline InstructionEffect
execute( CPUModel& cpu,
         isa::InstructionBits bits,
         isa::Operation op)
{
    isa::Instruction insn( bits);
    switch ( op )
    {
        #define DECLARE_INSN( name_, match_, mask_) \
            case isa::Operation::instr_ ## name_: return execute_ ## name_( cpu, insn);

        #include "riscv-opcodes/encoding.out.h"

        #undef DECLARE_INSN

        default:
        {
            RVI_UNREACHABLE();
        }
    }
}

} // ! namespace insn
} // ! namespace rvi

#endif // ! RVI_INSN_DISPATCH_HH__
