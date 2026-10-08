#ifndef RISCV_INTERPRETER_CPU_MODEL_HH__
#define RISCV_INTERPRETER_CPU_MODEL_HH__

#include <cstddef>
#include <cstdint>

#include "memory.hh"
#include "debug.hh"

namespace riscv
{

class CPUModel
{
public:
    static constexpr std::size_t kProgrammerRegistersNumber = 32;

public:
    uint32_t
    getX( unsigned n) const
    {
        RVI_ASSERT( n < kProgrammerRegistersNumber);
        return x_[n];
    }
    void
    setX( unsigned n,
          uint32_t value)
    {
        RVI_ASSERT( n < kProgrammerRegistersNumber);
        if ( n != 0 )
        {
            x_[n] = value;
        }
    }

    uint32_t
    getPC() const
    {
        return pc_;
    }

    void
    setPC( uint32_t value)
    {
        pc_ = value;
    }

    mem::Memory&
    memory()
    {
        return memory_;
    }

private:
    uint32_t x_[kProgrammerRegistersNumber]{};
    uint32_t pc_ = 0;
    mem::Memory memory_;

};

} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_CPU_MODEL_HH__
