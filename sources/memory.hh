#ifndef RISCV_INTERPRETER_MEMORY_HH__
#define RISCV_INTERPRETER_MEMORY_HH__

#include <cstddef>
#include <cstdint>
#include <limits>

namespace riscv
{
namespace mem
{

constexpr std::size_t kMemorySize = UINT32_MAX;

class Memory
{
public:
    void load( uint32_t addr, uint32_t size, void* data);
    void store( uint32_t addr, uint32_t size, const void* data);

    bool allocate( uint32_t addr, uint32_t size);

private:
};

} // ! namespace mem
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_MEMORY_HH__
