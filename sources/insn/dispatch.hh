#ifndef RVI_INSN_DISPATCH_HH__
#define RVI_INSN_DISPATCH_HH__

#include "helpers/error.hh"
#include "machine/cpu.hh"
#include "isa/decoder.hh"

namespace rvi
{
namespace insn
{

enum class StepResult
{
    Continue,
    Breakpoint,
    EnvironmentCall,
};

#define DECLARE_INSN( name_, match_, mask_) \
    Expected<StepResult> execute_ ## name_( CPUModel& cpu, isa::Instruction insn);

#include "riscv-opcodes/encoding.out.h"

#undef DECLARE_INSN

inline Expected<StepResult>
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
