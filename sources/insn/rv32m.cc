#include "insn/execute.hh"
#include "insn/helpers.hh"

namespace riscv
{
namespace insn
{

Expected<StepResult>
execute_mul( CPUModel& c,
             isa::Instruction i)
{
    uint64_t prod = uint64_t( c.getX( i.rs1())) * uint64_t( c.getX( i.rs2()));
    c.setX( i.rd(), low32( prod));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_mulh( CPUModel& c,
              isa::Instruction i)
{
    int64_t product = toSigned64From32( c.getX( i.rs1())) * toSigned64From32( c.getX( i.rs2()));
    c.setX( i.rd(), high32( asBits( product)));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_mulhsu( CPUModel& c,
                isa::Instruction i)
{
    int64_t product = toSigned64From32( c.getX( i.rs1())) * int64_t( c.getX( i.rs2()));
    c.setX( i.rd(), high32( asBits( product)));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_mulhu( CPUModel& c,
               isa::Instruction i)
{
    uint64_t product = uint64_t( c.getX( i.rs1())) * uint64_t( c.getX( i.rs2()));
    c.setX( i.rd(), high32( product));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_div( CPUModel& c,
             isa::Instruction i)
{
    uint32_t dividend = c.getX( i.rs1());
    uint32_t divisor = c.getX( i.rs2());
    c.setX( i.rd(), divideSigned( dividend, divisor));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_divu( CPUModel& c,
              isa::Instruction i)
{
    uint32_t dividend = c.getX( i.rs1());
    uint32_t divisor = c.getX( i.rs2());
    uint32_t quotient = divisor == 0
                        ? std::numeric_limits<uint32_t>::max()
                        : dividend / divisor;
    c.setX( i.rd(), quotient);
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_rem( CPUModel& c,
             isa::Instruction i)
{
    uint32_t dividend = c.getX( i.rs1());
    uint32_t divisor = c.getX( i.rs2());
    c.setX( i.rd(), remainderSigned( dividend, divisor));
    advancePC( c);
    return StepResult::Continue;
}

Expected<StepResult>
execute_remu( CPUModel& c,
              isa::Instruction i)
{
    uint32_t dividend = c.getX( i.rs1());
    uint32_t divisor = c.getX( i.rs2());
    uint32_t remainder = divisor == 0 ? dividend : dividend % divisor;
    c.setX( i.rd(), remainder);
    advancePC( c);
    return StepResult::Continue;
}

} // ! namespace insn
} // ! namespace riscv


// Expected<StepResult>
// execute_mul( CPUModel& cpu,
//              isa::Instruction insn)
// {
//     uint64_t product = static_cast<uint64_t>( cpu.getX( insn.rs1()))
//                        * static_cast<uint64_t>( cpu.getX( insn.rs2()));
//     cpu.setX( insn.rd(), static_cast<uint32_t>( product));
//     advancePC( cpu);
//     return StepResult::Continue;
// }
// 
// Expected<StepResult>
// execute_mulh( CPUModel& cpu,
//               isa::Instruction insn)
// {
//     uint32_t product = multiplyHighSigned( cpu.getX( insn.rs1()), cpu.getX( insn.rs2()));
//     cpu.setX( insn.rd(), product);
//     advancePC( cpu);
//     return StepResult::Continue;
// }
// 
// Expected<StepResult>
// execute_mulhsu( CPUModel& cpu,
//                 isa::Instruction insn)
// {
//     uint32_t product = multiplyHighSignedUnsigned( cpu.getX( insn.rs1()),
//                                                    cpu.getX( insn.rs2()));
//     cpu.setX( insn.rd(), product);
//     advancePC( cpu);
//     return StepResult::Continue;
// }
// 
// Expected<StepResult>
// execute_mulhu( CPUModel& cpu,
//                isa::Instruction insn)
// {
//     uint32_t product = multiplyHighUnsigned( cpu.getX( insn.rs1()), cpu.getX( insn.rs2()));
//     cpu.setX( insn.rd(), product);
//     advancePC( cpu);
//     return StepResult::Continue;
// }

// Expected<StepResult>
// execute_rem( CPUModel& cpu,
//              isa::Instruction insn)
// {
//     uint32_t remainder = remainderSigned( cpu.getX( insn.rs1()), cpu.getX( insn.rs2()));
//     cpu.setX( insn.rd(), remainder);
//     advancePC( cpu);
//     return StepResult::Continue;
// }
// 
// Expected<StepResult>
// execute_remu( CPUModel& cpu,
//               isa::Instruction insn)
// {
//     uint32_t dividend = cpu.getX( insn.rs1());
//     uint32_t divisor = cpu.getX( insn.rs2());
//     uint32_t remainder = divisor == 0 ? dividend : dividend % divisor;
//     cpu.setX( insn.rd(), remainder);
//     advancePC( cpu);
//     return StepResult::Continue;
// }
