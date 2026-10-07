#ifndef RISCV_INTERPRETER_ERRORS_HH__
#define RISCV_INTERPRETER_ERRORS_HH__

#include <string>
#include <variant>
#include <cassert>

#include "spdlog/spdlog.h"

namespace riscv
{

enum class ErrorCode
{

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
        T* value = std::get_if<T>( data_);
        assert( value != nullptr);
        return *value;
    }

    const T&
    value() const &
    {
        const T* value = std::get_if<T>( data_);
        assert( value != nullptr);
        return *value;
    }

    Error&
    error() &
    {
        Error* error = std::get_if<Error>( data_);
        assert( error != nullptr);
        return *error;
    }

    const Error&
    error() const &
    {
        const Error* error = std::get_if<Error>( data_);
        assert( error != nullptr);
        return *error;
    }

private:
    std::variant<T, Error> data_;

};

} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_ERRORS_HH__
