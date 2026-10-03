#ifndef RISCV_INTERPRETER_CPU_MODEL_HH__
#define RISCV_INTERPRETER_CPU_MODEL_HH__

#include <array>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <span>

namespace riscv
{

class CPUModel
{
public:
    uint32_t
    getRegister( uint32_t num) const
    {
        assert( num < kProgrammerRegistersCount);
        return x_registers_[num];
    }

    void
    setRegister( uint32_t num,
                 uint32_t value)
    {
        assert( num < kProgrammerRegistersCount);
        if ( num == 0 )
        {
            // Do nothing, change of x0 is prohibited.
        } else
        {
            x_registers_[num] = value;
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

    std::span<std::byte>
    getMemory( uint32_t offset, uint32_t width)
    {
        assert( offset + width < kMemorySize);
        return std::span<std::byte>( memory_.data() + offset, width);
    }

    std::span<const std::byte>
    getMemory( uint32_t offset, uint32_t width) const
    {
        assert( offset + width < kMemorySize);
        return std::span<const std::byte>( memory_.data() + offset, width);
    }

private:
    static constexpr std::size_t kProgrammerRegistersCount = 32;
    static constexpr std::size_t kMemorySize = 0x2000;

private:
    std::array<uint32_t, kProgrammerRegistersCount> x_registers_;
    uint32_t pc_;
    std::array<std::byte, kMemorySize> memory_;

};

} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_CPU_MODEL_HH__
