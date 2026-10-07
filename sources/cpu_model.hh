#ifndef RISCV_INTERPRETER_CPU_MODEL_HH__
#define RISCV_INTERPRETER_CPU_MODEL_HH__

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <memory>

namespace riscv
{

class CPUModel
{
public:
    static constexpr std::size_t kProgrammerRegistersNumber = 32;
    static constexpr std::size_t kMemorySize = 0x2000;

public:
    uint32_t
    getX( unsigned n) const
    {
        assert( n < kProgrammerRegistersNumber);
        return x_[n];
    }
    void
    setX( unsigned n,
          uint32_t value)
    {
        assert( n < kProgrammerRegistersNumber);
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

    void
    store( uint32_t offset,
           uint32_t size,
           const void* data)
    {
        assert( offset <= kMemorySize);
        assert( size <= kMemorySize - offset);
        std::memcpy( memory_.get() + offset, data, size);
    }

    void
    load( uint32_t offset,
          uint32_t size,
          void* data)
    {
        assert( offset <= kMemorySize);
        assert( size <= kMemorySize - offset);
        std::memcpy( data, memory_.get() + offset, size);
    }

private:
    uint32_t x_[kProgrammerRegistersNumber]{};
    uint32_t pc_ = 0;
    std::unique_ptr<std::byte[]> memory_ = std::make_unique<std::byte[]>( kMemorySize);

};

} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_CPU_MODEL_HH__
