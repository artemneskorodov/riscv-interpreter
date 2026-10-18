#include "insn/dispatch.hh"
#include "insn/helpers.hh"

namespace rvi
{
namespace insn
{

namespace
{

int64_t
toSigned64From32( uint32_t bits)
{
    if ( bits & (uint32_t( 1) << 31) )
    {
        return static_cast<int64_t>( bits) - (int64_t( 1) << 32);
    } else
    {
        return static_cast<int64_t>( bits);
    }
}

uint32_t
low32( uint64_t bits)
{
    uint64_t mask = uint64_t( std::numeric_limits<uint32_t>::max());
    return static_cast<uint32_t>( bits & mask);
}

uint32_t
high32( uint64_t bits)
{
    return static_cast<uint32_t>( bits >> 32);
}

uint64_t
asBits( int64_t value)
{
    return std::bit_cast<uint64_t>( value);
}

uint32_t
divideSigned( uint32_t dividend,
              uint32_t divisor)
{
    if ( divisor == 0 )
    {
        return std::numeric_limits<uint32_t>::max();
    }

    int64_t lhs = static_cast<int64_t>( asSigned( dividend));
    int64_t rhs = static_cast<int64_t>( asSigned( divisor));
    return static_cast<uint32_t>( lhs / rhs);
}

uint32_t
remainderSigned( uint32_t dividend,
                 uint32_t divisor)
{
    if ( divisor == 0 )
    {
        return dividend;
    }

    int64_t lhs = static_cast<int64_t>( asSigned( dividend));
    int64_t rhs = static_cast<int64_t>( asSigned( divisor));
    return static_cast<uint32_t>( lhs % rhs);
}

} // ! anonymous namespace

InstructionEffect
execute_mul( CPUModel& c,
             isa::Instruction i)
{
    uint64_t prod = uint64_t( c.getX( i.rs1())) * uint64_t( c.getX( i.rs2()));
    c.setX( i.rd(), low32( prod));
    return {};
}

InstructionEffect
execute_mulh( CPUModel& c,
              isa::Instruction i)
{
    int64_t product = toSigned64From32( c.getX( i.rs1())) * toSigned64From32( c.getX( i.rs2()));
    c.setX( i.rd(), high32( asBits( product)));
    return {};
}

InstructionEffect
execute_mulhsu( CPUModel& c,
                isa::Instruction i)
{
    int64_t product = toSigned64From32( c.getX( i.rs1())) * int64_t( c.getX( i.rs2()));
    c.setX( i.rd(), high32( asBits( product)));
    return {};
}

InstructionEffect
execute_mulhu( CPUModel& c,
               isa::Instruction i)
{
    uint64_t product = uint64_t( c.getX( i.rs1())) * uint64_t( c.getX( i.rs2()));
    c.setX( i.rd(), high32( product));
    return {};
}

InstructionEffect
execute_div( CPUModel& c,
             isa::Instruction i)
{
    uint32_t dividend = c.getX( i.rs1());
    uint32_t divisor = c.getX( i.rs2());
    c.setX( i.rd(), divideSigned( dividend, divisor));
    return {};
}

InstructionEffect
execute_divu( CPUModel& c,
              isa::Instruction i)
{
    uint32_t dividend = c.getX( i.rs1());
    uint32_t divisor = c.getX( i.rs2());
    uint32_t quotient = divisor == 0
                        ? std::numeric_limits<uint32_t>::max()
                        : dividend / divisor;
    c.setX( i.rd(), quotient);
    return {};
}

InstructionEffect
execute_rem( CPUModel& c,
             isa::Instruction i)
{
    uint32_t dividend = c.getX( i.rs1());
    uint32_t divisor = c.getX( i.rs2());
    c.setX( i.rd(), remainderSigned( dividend, divisor));
    return {};
}

InstructionEffect
execute_remu( CPUModel& c,
              isa::Instruction i)
{
    uint32_t dividend = c.getX( i.rs1());
    uint32_t divisor = c.getX( i.rs2());
    uint32_t remainder = divisor == 0 ? dividend : dividend % divisor;
    c.setX( i.rd(), remainder);
    return {};
}

} // ! namespace insn
} // ! namespace rvi
