#ifndef RISCV_INTERPRETER_CPU_MODEL_HH__
#define RISCV_INTERPRETER_CPU_MODEL_HH__

#include <cstddef>
#include <cstdint>
#include <string>

#include "memory.hh"
#include "debug.hh"
#include "bits.hh"

namespace riscv
{

class CPUModel
{
public:
    static constexpr std::size_t kProgrammerRegistersNumber = 32;
    static constexpr std::size_t kFloatingPointRegistersNumber = 32;

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

    uint32_t
    getF( unsigned n) const
    {
        RVI_ASSERT( n < kFloatingPointRegistersNumber);
        return f_[n];
    }

    void
    setF( unsigned n,
          uint32_t value)
    {
        RVI_ASSERT( n < kFloatingPointRegistersNumber);
        f_[n] = value;
    }

    uint32_t
    getFCSR() const
    {
        return fcsr_;
    }

    void
    setFCSR( uint32_t value)
    {
        RVI_ASSERT( (value & ~0xffu) == 0);
        fcsr_ = value;
    }

    void
    addFCSRFlags( uint32_t flags)
    {
        RVI_ASSERT( (flags & ~0x1f) == 0);
        fcsr_ |= flags;
    }

public:
    std::string toStr() const;

private:
    uint32_t x_[kProgrammerRegistersNumber]{};
    uint32_t f_[kFloatingPointRegistersNumber]{};
    uint32_t pc_ = 0;
    uint32_t fcsr_ = 0;
    mem::Memory memory_;

};

} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_CPU_MODEL_HH__
