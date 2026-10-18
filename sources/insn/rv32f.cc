#include <bit>
#include <functional>
#include <optional>

#include "insn/dispatch.hh"
#include "insn/helpers.hh"

extern "C"
{
#include "softfloat.h"
}

namespace rvi
{
namespace insn
{

namespace
{

constexpr uint32_t kCanonicalNaN = 0x7fc00000u;
constexpr uint32_t kSignBit      = uint32_t( 1) << 31;

float32_t
toFloat( uint32_t bits)
{
    return float32_t{ .v = bits};
}

uint8_t
toSoftFloatRoundingMode( uint32_t rm)
{
    RVI_ASSERT( rm <= 0x4);

    switch ( rm )
    {
        case 0x0: return softfloat_round_near_even;
        case 0x1: return softfloat_round_minMag;
        case 0x2: return softfloat_round_min;
        case 0x3: return softfloat_round_max;
        case 0x4: return softfloat_round_near_maxMag;
        default:
        {
            RVI_UNREACHABLE();
        }
    }
}

uint32_t
getFRM( uint32_t fcsr)
{
    return bits::getField<5, 7>( fcsr);
}

uint32_t
getFFlags( uint32_t fcsr)
{
    return bits::getField<0, 4>( fcsr);
}

uint32_t
getFCSR( uint32_t frm,
         uint32_t fflags)
{
    RVI_ASSERT( (frm & ~0x3) == 0);
    RVI_ASSERT( (fflags &~0x1f) == 0);

    return fflags | (frm << 5);
}

std::optional<uint8_t>
resolveRoundingMode( const CPUModel& cpu,
                     isa::Instruction insn)
{
    if ( insn.rm() <= 0x4 )
    {
        return toSoftFloatRoundingMode( insn.rm());
    } else if ( insn.rm() == 0x7 )
    {
        return toSoftFloatRoundingMode( getFRM( cpu.getFCSR()));
    }

    return std::nullopt;
}

void
beginFloatingPointOperation( uint8_t rounding_mode)
{
    // Setting rounding mode of softfloat to currently selected by instruction.
    softfloat_roundingMode = rounding_mode;

    // Resetting exception flags.
    softfloat_exceptionFlags = 0;

    //
    // According to documentation of F extension for RISCV:
    //  "tininess is detected after rounding".
    //
    softfloat_detectTininess = softfloat_tininess_afterRounding;
}

void
endFloatingPointOperation( CPUModel& c)
{
    bool nx = !!(softfloat_exceptionFlags & softfloat_flag_inexact);
    bool uf = !!(softfloat_exceptionFlags & softfloat_flag_underflow);
    bool of = !!(softfloat_exceptionFlags & softfloat_flag_overflow);
    bool dz = !!(softfloat_exceptionFlags & softfloat_flag_infinite);
    bool nv = !!(softfloat_exceptionFlags & softfloat_flag_invalid);

    uint32_t flags = ((nx ? 1u : 0) << 0)
                     | ((uf ? 1u : 0) << 1)
                     | ((of ? 1u : 0) << 2)
                     | ((dz ? 1u : 0) << 3)
                     | ((nv ? 1u : 0) << 4);
    uint32_t old_rm = getFRM( c.getFCSR());
    c.setFCSR( getFCSR( old_rm, flags));
}

template<typename Operation>
InstructionEffect
executeFloatingPointOperation( CPUModel& c,
                               isa::Instruction i,
                               Operation operation)
{
    std::optional<uint8_t> rm = resolveRoundingMode( c, i);
    if ( !rm.has_value() )
    {
        return InstructionEffect{
            .trap = isa::Trap{
                .cause = isa::ExceptionCause::IllegalInstruction,
                .value = i.bits(),
            },
        };
    }

    beginFloatingPointOperation( rm.value());
    float32_t result = operation();
    endFloatingPointOperation( c);

    c.setF( i.rd(), result.v);

    return {};
}

} // ! anonymous namespace

//
// Computing instructions with two operands
//

namespace
{

InstructionEffect
executeComputingInsn( CPUModel& c,
                      isa::Instruction i,
                      std::function<float32_t( float32_t, float32_t)> f)
{
    float32_t f1 = toFloat( c.getF( i.rs1()));
    float32_t f2 = toFloat( c.getF( i.rs2()));

    auto op = [&] { return f( f1, f2); };
    return executeFloatingPointOperation( c, i, op);
}

} // ! anonymous namespace

InstructionEffect
execute_fadd_s( CPUModel& c,
                isa::Instruction i)
{
    return executeComputingInsn( c, i, f32_add);
}

InstructionEffect
execute_fsub_s( CPUModel& c,
                isa::Instruction i)
{
    return executeComputingInsn( c, i, f32_sub);
}

InstructionEffect
execute_fmul_s( CPUModel& c,
                isa::Instruction i)
{
    return executeComputingInsn( c, i, f32_mul);
}

InstructionEffect
execute_fdiv_s( CPUModel& c,
                isa::Instruction i)
{
    return executeComputingInsn( c, i, f32_div);
}

//
// Square root
//

InstructionEffect
execute_fsqrt_s( CPUModel& c,
                 isa::Instruction i)
{
    float32_t f1 = toFloat( c.getF( i.rs1()));
    return executeFloatingPointOperation( c, i, [f1] { return f32_sqrt( f1); });
}

//
// Computing fused multiply add instructions
//

namespace
{

InstructionEffect
executeFusedMulAdd( CPUModel& c,
                    isa::Instruction i,
                    bool mul_neg,
                    bool third_neg)
{
    float32_t f1 = toFloat( c.getF( i.rs1()));
    float32_t f2 = toFloat( c.getF( i.rs2()));
    float32_t f3 = toFloat( c.getF( i.rs3()));

    auto op = [&] {
        if ( mul_neg ) f1.v = f1.v ^ kSignBit;
        if ( third_neg ) f3.v = f3.v ^ kSignBit;
        return f32_mulAdd( f1, f2, f3);
    };
    return executeFloatingPointOperation( c, i, op);
}

} // ! anonymous namespace

InstructionEffect
execute_fmadd_s( CPUModel& c,
                 isa::Instruction i)
{
    return executeFusedMulAdd( c, i, false, false);
}

InstructionEffect
execute_fmsub_s( CPUModel& c,
                 isa::Instruction i)
{
    return executeFusedMulAdd( c, i, false, true);
}

InstructionEffect
execute_fnmsub_s( CPUModel& c,
                  isa::Instruction i)
{
    return executeFusedMulAdd( c, i, true, false);
}

InstructionEffect
execute_fnmadd_s( CPUModel& c,
                  isa::Instruction i)
{
    return executeFusedMulAdd( c, i, true, true);
}

//
// Sing change instructions
//

namespace
{

InstructionEffect
executeSignChangeInsn( CPUModel& c,
                       isa::Instruction i,
                       std::function<uint32_t( uint32_t, uint32_t)> sign)
{
    uint32_t rs1 = c.getF( i.rs1());
    uint32_t rs2 = c.getF( i.rs2());

    uint32_t sign_bit = sign( rs1, rs2);
    uint32_t except_sign_bit = rs1 & ~kSignBit;
    uint32_t result = sign_bit | except_sign_bit;

    c.setF( i.rd(), result);
    return {};
}

} // ! anonymous namespace

InstructionEffect
execute_fsgnj_s( CPUModel& c,
                 isa::Instruction i)
{
    auto sign = []( uint32_t /*rs1*/, uint32_t rs2) { return rs2 & kSignBit; };
    return executeSignChangeInsn( c, i, sign);
}

InstructionEffect
execute_fsgnjn_s( CPUModel& c,
                  isa::Instruction i)
{
    auto sign = []( uint32_t /*rs1*/, uint32_t rs2) { return ~rs2 & kSignBit; };
    return executeSignChangeInsn( c, i, sign);
}

InstructionEffect
execute_fsgnjx_s( CPUModel& c,
                  isa::Instruction i)
{
    auto sign = []( uint32_t rs1, uint32_t rs2) { return (rs1 & rs2) & kSignBit; };
    return executeSignChangeInsn( c, i, sign);
}

//
// Floating point control and state register access instructions
//

namespace
{

InstructionEffect
executeChangeRoundingMode( CPUModel& c,
                           isa::Instruction i,
                           uint32_t rm)
{
    uint32_t new_fcsr_rm =rm & 0x7;
    uint32_t old_fcsr_flags = getFFlags( c.getFCSR());
    c.setX( i.rd(), getFRM( c.getFCSR()));
    c.setFCSR( getFCSR( new_fcsr_rm, old_fcsr_flags));

    return {};
}

InstructionEffect
executeChangeFlags( CPUModel& c,
                    isa::Instruction i,
                    uint32_t flags)
{
    uint32_t new_fcsr_flags = flags & 0x1f;
    uint32_t old_fcsr_rm = getFRM( c.getFCSR());
    c.setX( i.rd(), getFFlags( c.getFCSR()));
    c.setFCSR( getFCSR( old_fcsr_rm, new_fcsr_flags));
    return {};
}

} // ! anonymous namespace

InstructionEffect
execute_frcsr( CPUModel& c,
               isa::Instruction i)
{
    c.setX( i.rd(), c.getFCSR());
    return {};
}

InstructionEffect
execute_fscsr( CPUModel& c,
               isa::Instruction i)
{
    uint32_t new_fcsr = c.getX( i.rs1());
    c.setX( i.rd(), c.getFCSR());
    c.setFCSR( new_fcsr);
    return {};
}

InstructionEffect
execute_frrm( CPUModel& c,
              isa::Instruction i)
{
    c.setX( i.rd(), getFRM( c.getFCSR()));
    return{};
}

InstructionEffect
execute_fsrm( CPUModel& c,
              isa::Instruction i)
{
    return executeChangeRoundingMode( c, i, c.getX( i.rs1()));
}

InstructionEffect
execute_fsrmi( CPUModel& c,
               isa::Instruction i)
{
    return executeChangeRoundingMode( c, i, i.rs1());
}

InstructionEffect
execute_frflags( CPUModel& c,
                 isa::Instruction i)
{
    c.setX( i.rd(), getFFlags( c.getFCSR()));
    return {};
}

InstructionEffect
execute_fsflags( CPUModel& c,
                 isa::Instruction i)
{
    return executeChangeFlags( c, i, c.getX( i.rs1()));
}

InstructionEffect
execute_fsflagsi( CPUModel& c,
                  isa::Instruction i)
{
    return executeChangeFlags( c, i, i.rs1());
}

//
// Floating point load and store instructions
//

InstructionEffect
execute_flw( CPUModel& c,
             isa::Instruction i)
{
    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>();
    machine::MemoryResult<uint32_t> value = c.memory().load<uint32_t>( addr);
    if ( !value.ok() )
    {
        return InstructionEffect{ .trap = value.error() };
    }
    c.setF( i.rd(), value.value());
    return {};
}

InstructionEffect
execute_fsw( CPUModel& c,
             isa::Instruction i)
{
    uint32_t addr = c.getX( i.rs1()) + i.imm<isa::ImmType::I, true>();
    uint32_t value = c.getF( i.rs2());
    machine::MemoryResult<void> r = c.memory().store<uint32_t>( addr, value);
    if ( !r.ok() )
    {
        return InstructionEffect{ .trap = r.error() };
    }
    return {};
}

//
// Converters instructions
//

InstructionEffect
execute_fcvt_w_s( CPUModel& c,
                  isa::Instruction i)
{
    std::optional<uint8_t> rm = resolveRoundingMode( c, i);
    if ( !rm.has_value() )
    {
        return InstructionEffect{
            .trap = isa::Trap{
                .cause = isa::ExceptionCause::IllegalInstruction,
                .value = i.bits(),
            },
        };
    }

    beginFloatingPointOperation( rm.value());
    int_fast32_t v = f32_to_i32( toFloat( c.getF( i.rs1())), rm.value(), true);
    endFloatingPointOperation( c);

    c.setX( i.rd(), v);
    return {};
}

InstructionEffect
execute_fcvt_wu_s( CPUModel& c,
                   isa::Instruction i)
{
    std::optional<uint8_t> rm = resolveRoundingMode( c, i);
    if ( !rm.has_value() )
    {
        return InstructionEffect{
            .trap = isa::Trap{
                .cause = isa::ExceptionCause::IllegalInstruction,
                .value = i.bits(),
            },
        };
    }

    beginFloatingPointOperation( rm.value());
    uint_fast32_t v = f32_to_ui32( toFloat( c.getF( i.rs1())), rm.value(), true);
    endFloatingPointOperation( c);

    c.setX( i.rd(), v);
    return {};
}

InstructionEffect
execute_fcvt_s_w( CPUModel& c,
                  isa::Instruction i)
{
    auto op = [&] { return i32_to_f32( c.getX( i.rs1())); };
    return executeFloatingPointOperation( c, i, op);
}

InstructionEffect
execute_fcvt_s_wu( CPUModel& c,
                   isa::Instruction i)
{
    auto op = [&] { return ui32_to_f32( c.getX( i.rs1())); };
    return executeFloatingPointOperation( c, i, op);
}

//
// Moving to x-registers instructions
//

InstructionEffect
execute_fmv_x_w( CPUModel& c,
                 isa::Instruction i)
{
    c.setX( i.rd(), c.getF( i.rs1()));
    return {};
}

InstructionEffect
execute_fmv_w_x( CPUModel& c,
                 isa::Instruction i)
{
    c.setF( i.rd(), c.getX( i.rs1()));
    return {};
}

//
// Compare instructions
//
namespace
{

InstructionEffect
executeCmpInsn( CPUModel& c,
                isa::Instruction i,
                std::function<bool( float32_t, float32_t)> comparator)
{
    beginFloatingPointOperation( 0); // Rounding mode does not matter
    bool result = comparator( toFloat( c.getF( i.rs1())),
                              toFloat( c.getF( i.rs2())));
    endFloatingPointOperation( c);
    c.setX( i.rd(), result ? 1u : 0u);
    return {};
}

} // ! anonymous namespace

InstructionEffect
execute_feq_s( CPUModel& c,
               isa::Instruction i)
{
    return executeCmpInsn( c, i, f32_eq);
}

InstructionEffect
execute_fle_s( CPUModel& c,
               isa::Instruction i)
{
    return executeCmpInsn( c, i, f32_le);
}

InstructionEffect
execute_flt_s( CPUModel& c,
               isa::Instruction i)
{
    return executeCmpInsn( c, i, f32_lt);
}

//
// Min and max instructions
//

namespace
{

bool
isNaN( uint32_t v)
{
    uint32_t sign = bits::getField<31, 31>( v);
    uint32_t exponent = bits::getField<23, 30>( v);
    return (sign == 0) && (exponent == 0xff);
}

bool
isQuiteNaN( uint32_t v)
{
    uint32_t quite_bit = bits::getField<22, 22>( v);
    return isNaN( v) && (quite_bit == 1);
}

bool
isSignallingNaN( uint32_t v)
{
    uint32_t quite_bit = bits::getField<22, 22>( v);
    return isNaN( v) && (quite_bit == 0);
}

bool
isZero( uint32_t v)
{
    return (v & ~kSignBit) == 0;
}

//
// fmin.s and fmax.s implement IEEE 754-201x minimumNumber and maximumNumber functions.
//
uint32_t
minimumMaximum( CPUModel& c,
                uint32_t lhs,
                uint32_t rhs,
                bool minimum)
{
    if ( isSignallingNaN( lhs) || isSignallingNaN( rhs) )
    {
        c.addFCSRFlags( 1u << 4); // Invalid operation
        // Return must be the same as in case of quite NaN's.
    }

    // Returning canonical NaN, if both operands are NaN.
    if ( isNaN( lhs) && isNaN( rhs) )
    {
        return kCanonicalNaN;
    }

    // Returning not NaN operand, if one of operands is NaN
    if ( isNaN( lhs) )
    {
        return rhs;
    }
    if ( isNaN( rhs) )
    {
        return lhs;
    }

    //
    // If both are zero, than (-0.0 < +0.0)
    //
    if ( isZero( lhs) && isZero( rhs) )
    {
        return minimum
               ? (lhs | rhs)
               : (lhs & rhs);
    }

    // Default floats compare
    bool l_lt_r = f32_lt( toFloat( lhs), toFloat( rhs));
    if ( minimum )
    {
        return l_lt_r ? lhs : rhs;
    } else
    {
        return l_lt_r ? rhs : lhs;
    }
}

} // ! anonymous namespace

InstructionEffect
execute_fmax_s( CPUModel& c,
                isa::Instruction i)
{
    uint32_t result = minimumMaximum( c, c.getF( i.rs1()), c.getF( i.rs2()), false);
    c.setF( i.rd(), result);
    return {};
}

InstructionEffect
execute_fmin_s( CPUModel& c,
                isa::Instruction i)
{
    uint32_t result = minimumMaximum( c, c.getF( i.rs1()), c.getF( i.rs2()), true);
    c.setF( i.rd(), result);
    return {};
}

//
// Classification of floating point number.
//

namespace
{

uint32_t
classify( uint32_t value)
{
    uint32_t sign = bits::getField<31, 31>( value);
    uint32_t exponent = bits::getField<23, 30>( value);
    uint32_t mantissa = bits::getField<0, 22>( value);

    if ( (exponent == 0) && (mantissa == 0) )
    {
        // +0.0 or -0.0
        if ( sign )
        {
            return (1u << 3); // -0.0
        } else
        {
            return (1u << 4); // +0.0
        }
    }

    if ( (exponent == 0xff) && (mantissa == 0) )
    {
        if ( sign )
        {
            return (1u << 0); // -inf
        } else
        {
            return (1u << 7); // +inf
        }
    }

    if ( isSignallingNaN( value) )
    {
        return (1u << 8);
    }
    if ( isQuiteNaN( value) )
    {
        return (1u << 9);
    }

    bool negative = !!(sign);
    bool subnormal = (exponent == 0);

    if ( negative )
    {
        return subnormal
               ? (1u << 2)  // Negative subnormal
               : (1u << 1); // Negative normal
    } else
    {
        return subnormal
               ? (1u << 5)  // Positive subnormal
               : (1u << 6); // Positive normal
    }
}

} // ! anonymous namespace

InstructionEffect
execute_fclass_s( CPUModel& c,
                  isa::Instruction i)
{
    c.setX( i.rd(), classify( c.getF( i.rs1())));
    return {};
}

} // ! namespace insn
} // ! namespace rvi
