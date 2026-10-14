#ifndef RISCV_INTERPRETER_INSNS_HELPERS_HH__
#define RISCV_INTERPRETER_INSNS_HELPERS_HH__

#include <cstdint>

namespace riscv
{
namespace insn
{

inline uint32_t
mulh( uint32_t lhs,
      uint32_t rhs)
{
    int64_t lhs64 = static_cast<int64_t>( static_cast<int32_t>( lhs));
    int64_t rhs64 = static_cast<int64_t>( static_cast<int32_t>( rhs));

    int64_t prod = lhs64 * rhs64;
    uint64_t bits = static_cast<uint64_t>( prod);
    return static_cast<uint32_t>( bits >> 32);
}

inline uint32_t
mulhsu( uint32_t lhs,
        uint32_t rhs)
{
    int64_t prod = static_cast<int64_t>( static_cast<int32_t>( lhs))
                   * static_cast<int64_t>( rhs);

    uint64_t bits = static_cast<uint64_t>( prod);
    return static_cast<uint32_t>( bits >> 32);
}

inline uint32_t
mulhu( uint32_t lhs,
       uint32_t rhs)
{
    uint64_t prod = static_cast<uint64_t>( lhs) * static_cast<uint64_t>( rhs);
    return static_cast<uint32_t>( prod >> 32);
}

inline uint32_t
mul( uint32_t lhs,
     uint32_t rhs)
{
    uint64_t prod = static_cast<uint64_t>( lhs) * static_cast<uint64_t>( rhs);
    return static_cast<uint32_t>( prod);
}

} // ! namespace insn
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_INSNS_HELPERS_HH__
