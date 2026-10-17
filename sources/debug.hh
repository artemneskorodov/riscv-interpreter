#ifndef RISCV_INTERPRETER_DEBUG_HH__
#define RISCV_INTERPRETER_DEBUG_HH__

#include <source_location>
#include <string>
#include <vector>
#include <cstddef>

#include "spdlog/spdlog.h"

#define RVI_TRACE(    ...) SPDLOG_TRACE(    __VA_ARGS__)
#define RVI_DEBUG(    ...) SPDLOG_DEBUG(    __VA_ARGS__)
#define RVI_INFO(     ...) SPDLOG_INFO(     __VA_ARGS__)
#define RVI_WARN(     ...) SPDLOG_WARN(     __VA_ARGS__)
#define RVI_ERROR(    ...) SPDLOG_ERROR(    __VA_ARGS__)
#define RVI_CRITICAL( ...) SPDLOG_CRITICAL( __VA_ARGS__)

//
// This must be called in main start. The spdlog compile time level is set with CMake,
// but spdlog runtime level is set to default info.
//
#define RVI_INIT_SPDLOG_LEVEL ::spdlog::set_level( spdlog::level::trace)

namespace riscv
{
namespace debug
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
}

inline void
onUnreachableReached( std::source_location location)
{
    RVI_CRITICAL( "Unreachable code reached at {}:{} in {}",
                  location.file_name(),
                  location.line(),
                  location.function_name());
}

} // ! namespace detail

inline std::string
memoryRangesToStr( const std::vector<std::pair<uint32_t, uint32_t>>& ranges)
{
    std::string result = "";
    for ( auto it = ranges.begin(); it != ranges.end(); ++it )
    {
        result += std::format( "[{:#x}:{:#x}){}",
                               it->first,
                               it->second,
                               (std::next( it) == ranges.end()) ? ""
                                                                : ", ");
    }
    return result;
}

} // ! namespace debug
} // ! namespace riscv

#if RVI_DEBUG_ENABLED
    #define RVI_ASSERT( expression_)                                                               \
    do                                                                                             \
    {                                                                                              \
        if ( !(expression_) )                                                                      \
        {                                                                                          \
            ::riscv::debug::detail::onAssertionFailed( #expression_,                               \
                                                       std::source_location::current());           \
            std::abort();                                                                          \
        }                                                                                          \
    } while ( 0 )

    #define RVI_UNREACHABLE()                                                                      \
    do                                                                                             \
    {                                                                                              \
        ::riscv::debug::detail::onUnreachableReached( std::source_location::current());            \
        std::abort();                                                                              \
    } while ( 0 )

    #define RVI_FAILURE( ...)                                                                      \
    do                                                                                             \
    {                                                                                              \
        RVI_CRITICAL( __VA_ARGS__);                                                                \
        ::std::abort();                                                                            \
    } while ( 0 )

#else // RVI_DEBUG_ENABLED
    #define RVI_ASSERT( condition) ((void)0)
    #define RVI_UNREACHABLE()      ((void)0)
    #define RVI_FAILURE( ...)      ((void)0)
#endif // RVI_DEBUG_ENABLED

#endif // ! RISCV_INTERPRETER_DEBUG_HH__
