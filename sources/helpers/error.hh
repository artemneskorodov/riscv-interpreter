#ifndef RVI_HELPERS_ERROR_HH__
#define RVI_HELPERS_ERROR_HH__

#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "helpers/diagnostics.hh"

namespace rvi
{

enum class InterpreterErrorCode
{
    MemoryRegionsOverlap = 1,
    InvalidMemoryAccess,
    ElfClassUnexpected,
    ElfEncodingUnexpected,
    ElfMachineUnexpected,
    ElfTypeUnexpected,
    ElfVersionUnexpected,
    ElfSegmentTypeUnexpected,
    ElfOpenFailed,
    NoElfFileSpecified,
};

struct InterpreterError
{
    InterpreterErrorCode code;
    std::string message;
};

template<typename T,
         typename E = InterpreterError>
class Expected
{
public:
    Expected( T value)
        : data_( std::move( value))
    {}

    Expected( E error)
        : data_( std::move( error))
    {}

    bool ok() const { return std::holds_alternative<T>( data_); }

    T&
    value() &
    {
        T* value = std::get_if<T>( &data_);
        RVI_ASSERT( value != nullptr);
        return *value;
    }

    const T&
    value() const &
    {
        const T* value = std::get_if<T>( &data_);
        RVI_ASSERT( value != nullptr);
        return *value;
    }

    E&
    error() &
    {
        E* error = std::get_if<E>( &data_);
        RVI_ASSERT( error != nullptr);
        return *error;
    }

    const E&
    error() const &
    {
        const E* error = std::get_if<E>( &data_);
        RVI_ASSERT( error != nullptr);
        return *error;
    }

private:
    std::variant<T, E> data_;

};

template<typename E>
class Expected<void, E>
{
public:
    Expected()
        : data_( std::nullopt)
    {}

    Expected( E error)
        : data_( std::move( error))
    {}

    bool ok() const { return !data_.has_value(); }

    E&
    error() &
    {
        RVI_ASSERT( data_.has_value());
        return *data_;
    }

    const E&
    error() const &
    {
        RVI_ASSERT( data_.has_value());
        return *data_;
    }

private:
    std::optional<E> data_;

};

template<typename T>
using InterpreterResult = Expected<T, InterpreterError>;

} // ! namespace rvi

#endif // ! RVI_HELPERS_ERROR_HH__
