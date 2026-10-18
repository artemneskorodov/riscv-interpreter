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

} // ! namespace insn
} // ! namespace rvi

#endif // ! RVI_INSN_HELPERS_HH__
