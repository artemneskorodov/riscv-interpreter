#include "insn/dispatch.hh"
#include "insn/helpers.hh"

namespace rvi
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
} // ! namespace rvi
