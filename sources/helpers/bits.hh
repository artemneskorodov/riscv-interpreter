#ifndef RVI_HELPERS_BITS_HH__
#define RVI_HELPERS_BITS_HH__

#include <concepts>
#include <limits>
#include <cassert>

namespace rvi
{
namespace bits
{

template<unsigned First,
         unsigned Last,
         std::unsigned_integral T>
constexpr T
getField( T value)
{
    constexpr unsigned bits = std::numeric_limits<T>::digits;
    static_assert( First <= Last);
    static_assert( Last < bits);

    constexpr unsigned width = Last - First + 1;
    constexpr T mask = std::numeric_limits<T>::max() >> (bits - width);
    return (value >> First) & mask;
}

template<std::unsigned_integral T>
constexpr T
signExtend( T value,
            unsigned width)
{
    constexpr unsigned bits = std::numeric_limits<T>::digits;

    assert( width > 0);
    assert( width <= bits);

    if ( width == bits )
    {
        return value;
    }

    const T sign_bit = T{1} << (width - 1);

    if ( value & sign_bit )
    {
        const T extendMask = ~((T{1} << width) - 1);
        value |= extendMask;
    }

    return value;
}

} // ! namespace bits
} // ! namespace rvi

#endif // ! RVI_HELPERS_BITS_HH__
