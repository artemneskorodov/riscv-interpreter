#ifndef RVI_HELPERS_ERROR_HH__
#define RVI_HELPERS_ERROR_HH__

#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "helpers/diagnostics.hh"

namespace rvi
{

enum class ErrorCode
{
    MemoryRegionsOverlap,
    InvalidMemoryAccess,
    ElfClassUnexpected,
    ElfEncodingUnexpected,
    ElfMachineUnexpected,
    ElfTypeUnexpected,
    ElfVersionUnexpected,
    ElfSegmentTypeUnexpected,
    ElfOpenFailed,
    InvalidInstruction,
    UnsupportedInstruction,
    NoElfFileSpecified,
};

struct Error
{
    ErrorCode error;
    std::string message;
};

template<typename T>
class Expected
{
public:
    Expected( T value)
        : data_( std::move( value))
    {}

    Expected( Error error)
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

    Error&
    error() &
    {
        Error* error = std::get_if<Error>( &data_);
        RVI_ASSERT( error != nullptr);
        return *error;
    }

    const Error&
    error() const &
    {
        const Error* error = std::get_if<Error>( &data_);
        RVI_ASSERT( error != nullptr);
        return *error;
    }

private:
    std::variant<T, Error> data_;

};

template<>
class Expected<void>
{
public:
    Expected()
        : data_( std::nullopt)
    {}

    Expected( Error error)
        : data_( std::move( error))
    {}

    bool ok() const { return !data_.has_value(); }

    Error&
    error() &
    {
        RVI_ASSERT( data_.has_value());
        return *data_;
    }

    const Error&
    error() const &
    {
        RVI_ASSERT( data_.has_value());
        return *data_;
    }

private:
    std::optional<Error> data_;

};

} // ! namespace rvi

#endif // ! RVI_HELPERS_ERROR_HH__
