#include <bit>

#include "insn/dispatch.hh"
#include "insn/helpers.hh"

namespace rvi
{
namespace insn
{

//
// Logical with negative
//

namespace
{

InstructionEffect
executeLogicalWithNegative( CPUModel& c,
                            isa::Instruction i,
                            std::function<uint32_t( uint32_t, uint32_t)> op)
{
    uint32_t rs1 = c.getX( i.rs1());
    uint32_t rs2 = c.getX( i.rs2());

    uint32_t result = op( rs1, ~rs2);

    c.setX( i.rd(), result);
    return {};
}

} // ! anonymous namespace

InstructionEffect
execute_andn( CPUModel& c,
              isa::Instruction i)
{
    auto op = []( uint32_t a, uint32_t b) { return (a & b); };
    return executeLogicalWithNegative( c, i, op);
}

InstructionEffect
execute_orn( CPUModel& c,
             isa::Instruction i)
{
    auto op = []( uint32_t a, uint32_t b) { return (a | b); };
    return executeLogicalWithNegative( c, i, op);
}

InstructionEffect
execute_xnor( CPUModel& c,
              isa::Instruction i)
{
    auto op = []( uint32_t a, uint32_t b) { return (a ^ b); };
    return executeLogicalWithNegative( c, i, op);
}

//
// Count leading/trailing zero bits
//

namespace
{

InstructionEffect
executeCountZeros( CPUModel& c,
                   isa::Instruction i,
                   std::function<int( uint32_t)> counter)
{
    uint32_t value = c.getX( i.rs1());
    int result = counter( value);
    RVI_ASSERT( (result >= 0) && (result <= 32));
    c.setX( i.rd(), static_cast<uint32_t>( result));
    return {};
}

}

InstructionEffect
execute_clz( CPUModel& c,
             isa::Instruction i)
{
    return executeCountZeros( c, i, std::countl_zero<uint32_t>);
}

InstructionEffect
execute_ctz( CPUModel& c,
             isa::Instruction i)
{
    return executeCountZeros( c, i, std::countr_zero<uint32_t>);
}

//
// Count population
//

InstructionEffect
execute_cpop( CPUModel& c,
              isa::Instruction i)
{
    uint32_t value = c.getX( i.rs1());
    uint32_t result = std::popcount( value);
    c.setX( i.rd(), result);
    return {};
}

//
// Integer minimum/maximum
//
namespace
{

InstructionEffect
executeMinMax( CPUModel& c,
               isa::Instruction i,
               bool is_signed,
               bool is_minimum)
{
    uint32_t lhs = c.getX( i.rs1());
    uint32_t rhs = c.getX( i.rs2());

    bool is_lhs;
    if ( is_minimum )
    {
        is_lhs = is_signed
                 ? (asSigned( lhs) < asSigned( rhs))
                 : (lhs < rhs);
    } else
    {
        is_lhs = is_signed
                 ? (asSigned( lhs) > asSigned( rhs))
                 : (lhs > rhs);
    }

    uint32_t result = is_lhs ? lhs : rhs;
    c.setX( i.rd(), result);
    return {};
}

} // ! anonymous namespace

InstructionEffect
execute_max( CPUModel& c,
             isa::Instruction i)
{
    return executeMinMax( c, i, true, false);
}

InstructionEffect
execute_maxu( CPUModel& c,
              isa::Instruction i)
{
    return executeMinMax( c, i, false, false);
}

InstructionEffect
execute_min( CPUModel& c,
             isa::Instruction i)
{
    return executeMinMax( c, i, true, true);
}

InstructionEffect
execute_minu( CPUModel& c,
              isa::Instruction i)
{
    return executeMinMax( c, i, false, true);
}

//
// Sign- and zero-extension
//

namespace
{

InstructionEffect
executeSignExtent( CPUModel& c,
                   isa::Instruction i,
                   uint32_t width)
{
    uint32_t value = c.getX( i.rs1());
    uint32_t result = bits::signExtend( value, width);
    c.setX( i.rd(), result);
    return {};
}

} // ! anonymous namespace

InstructionEffect
execute_sext_b( CPUModel& c,
                isa::Instruction i)
{
    return executeSignExtent( c, i, 8);
}

InstructionEffect
execute_sext_h( CPUModel& c,
                isa::Instruction i)
{
    return executeSignExtent( c, i, 16);
}

InstructionEffect
execute_zext_h_rv32( CPUModel& c,
                     isa::Instruction i)
{
    uint32_t value = c.getX( i.rs1());
    uint32_t result = value & 0x0000ffff;
    c.setX( i.rd(), result);
    return {};
}

//
// Bitwise rotation
//

namespace
{

template<typename Rotator>
InstructionEffect
executeRotate( CPUModel& c,
               isa::Instruction i,
               uint32_t shamt,
               Rotator rotator)
{
    uint32_t value = c.getX( i.rs1());
    shamt = lowerFive( shamt);
    uint32_t result = rotator( value, static_cast<int>( shamt));
    c.setX( i.rd(), result);
    return {};
}

} // ! anonymous namespace

InstructionEffect
execute_rol( CPUModel& c,
             isa::Instruction i)
{
    return executeRotate( c, i, c.getX( i.rs2()), std::rotl<uint32_t>);
}

InstructionEffect
execute_ror( CPUModel& c,
             isa::Instruction i)
{
    return executeRotate( c, i, c.getX( i.rs2()), std::rotr<uint32_t>);
}

InstructionEffect
execute_rori_rv32( CPUModel& c,
                   isa::Instruction i)
{
    return executeRotate( c, i, i.rs2(), std::rotr<uint32_t>);
}

//
// OR Combine
//

InstructionEffect
execute_orc_b( CPUModel& c,
               isa::Instruction i)
{
    uint32_t src = c.getX( i.rs1());
    uint32_t result = 0;
    for ( uint32_t i = 0; i != sizeof( src) / sizeof( uint8_t); ++i )
    {
        uint32_t mask = 0xff << (i * 8);
        if ( src & mask )
        {
            result |= (mask);
        }
    }
    c.setX( i.rd(), result);
    return {};
}

//
// Byte-reverse register
//

InstructionEffect
execute_rev8_rv32( CPUModel& c,
                   isa::Instruction i)
{
    uint32_t src = c.getX( i.rs1());
    uint32_t result = std::byteswap( src);
    c.setX( i.rd(), result);
    return {};
}

} // ! namespace insn
} // ! namespace rvi
