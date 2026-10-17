#ifndef RVI_INSN_HELPERS_HH__
#define RVI_INSN_HELPERS_HH__

#include <bit>
#include <cstdint>
#include <limits>

#include "machine/cpu.hh"
#include "isa/decoder.hh"
#include "helpers/diagnostics.hh"
#include "helpers/bits.hh"

namespace rvi
{
namespace insn
{

inline void
advancePC( CPUModel& cpu)
{
    cpu.setPC( cpu.getPC() + sizeof( isa::InstructionBits));
}

inline int32_t
asSigned( uint32_t value)
{
    return std::bit_cast<int32_t>( value);
}

inline uint32_t
lowerFive( uint32_t value)
{
    return value & 0x1f;
}

inline uint32_t
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

inline int64_t
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

inline uint32_t
low32( uint64_t bits)
{
    uint64_t mask = uint64_t( std::numeric_limits<uint32_t>::max());
    return static_cast<uint32_t>( bits & mask);
}

inline uint32_t
high32( uint64_t bits)
{
    return static_cast<uint32_t>( bits >> 32);
}

inline uint64_t
asBits( int64_t value)
{
    return std::bit_cast<uint64_t>( value);
}

inline uint32_t
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

inline uint32_t
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

} // ! namespace insn
} // ! namespace rvi

#endif // ! RVI_INSN_HELPERS_HH__
