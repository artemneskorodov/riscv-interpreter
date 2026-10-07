#ifndef RISCV_INTERPRETER_DEBUG_HH__
#define RISCV_INTERPRETER_DEBUG_HH__

#include <source_location>

#include "spdlog/spdlog.h"

#define RVI_TRACE(    ...) SPDLOG_TRACE(    __VA_ARGS__)
#define RVI_DEBUG(    ...) SPDLOG_DEBUG(    __VA_ARGS__)
#define RVI_INFO(     ...) SPDLOG_INFO(     __VA_ARGS__)
#define RVI_WARN(     ...) SPDLOG_WARN(     __VA_ARGS__)
#define RVI_ERROR(    ...) SPDLOG_ERROR(    __VA_ARGS__)
#define RVI_CRITICAL( ...) SPDLOG_CRITICAL( __VA_ARGS__)

namespace riscv
{
namespace detail
{

inline void
onAssertionFailed( const char* expression,
                   std::source_location location)
{
    RVI_CRITICAL( "assertion failed: {} at {}:{} in {}",
                  expression,
                  location.file_name(),
                  location.line(),
                  location.function_name());
    std::abort();
}

inline void
onUnreachableReached( std::source_location location)
{
    RVI_CRITICAL( "Unreachable code reached at {}:{} in {}",
                  location.file_name(),
                  location.line(),
                  location.function_name());
    std::abort();
}

} // ! namespace detail
} // ! namespace riscv

#if RVI_DEBUG_ENABLED
    #define RVI_ASSERT( expression_)                                                               \
    do                                                                                             \
    {                                                                                              \
        if ( !(expression_) )                                                                      \
        {                                                                                          \
            ::riscv::detail::onAssertionFailed( #expression_, std::source_location::current());    \
        }                                                                                          \
    } while ( 0 )

    #define RVI_UNREACHABLE()                                                                      \
    do                                                                                             \
    {                                                                                              \
        ::riscv::detail::onUnreachableReached( std::source_location::current());                   \
    } while ( 0 )

#else // RVI_DEBUG_ENABLED
    #define RVI_ASSERT( condition) ((void)0)
    #define RVI_UNREACHABLE() ((void)0)
#endif // RVI_DEBUG_ENABLED

#endif // ! RISCV_INTERPRETER_DEBUG_HH__
