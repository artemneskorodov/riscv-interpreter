#ifndef RISCV_INTERPRETER_MEMORY_HH__
#define RISCV_INTERPRETER_MEMORY_HH__

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>
#include <concepts>

#include "errors.hh"

namespace riscv
{
namespace mem
{

class MemoryPermissions
{
public:
    enum Permissions
    {
        READ    = (1 << 0),
        WRITE   = (1 << 1),
        EXECUTE = (1 << 2),
    };

    MemoryPermissions( uint8_t flags)
        : flags_( flags)
    {}

    bool Read()    const { return !!(flags_ & Permissions::READ);    }
    bool Write()   const { return !!(flags_ & Permissions::WRITE);   }
    bool Execute() const { return !!(flags_ & Permissions::EXECUTE); }

private:
    uint8_t flags_;

};

struct MemoryRegion
{
    MemoryRegion( uint32_t base,
                  uint32_t size,
                  MemoryPermissions permissions)
        : base( base),
          size( size),
          permissions( permissions),
          data( std::make_unique<std::byte[]>( size))
    {}

    uint32_t base;
    uint32_t size;
    MemoryPermissions permissions;
    std::unique_ptr<std::byte[]> data;

    bool
    contains( uint32_t addr,
              uint32_t size) const
    {
        return (addr >= base) && (addr + size <= base + size);
    }

    bool
    overlaps( uint32_t addr,
              uint32_t size) const
    {
        return ( (addr >= base) && (addr < base + size) )
               || ( (addr + size >= base) && (addr + size < base + size) );
    }
};

template<typename T>
concept MemoryAccessType = std::same_as<T, uint8_t>
                           || std::same_as<T, uint16_t>
                           || std::same_as<T, uint32_t>;

class Memory
{
public:
    Expected<void> allocate( uint32_t base, uint32_t size, MemoryPermissions permissions);

    Expected<uint8_t>  load8(  uint32_t addr) { return loadImpl<uint8_t>( addr); }
    Expected<uint16_t> load16( uint32_t addr) { return loadImpl<uint16_t>( addr); }
    Expected<uint32_t> load32( uint32_t addr) { return loadImpl<uint32_t>( addr); }

    Expected<uint32_t> loadExecutable( uint32_t addr) { return loadImpl<uint32_t>( addr, true); }

    Expected<void> store8(  uint32_t addr, uint8_t  value) { return storeImpl<uint8_t>( addr, value); }
    Expected<void> store16( uint32_t addr, uint16_t value) { return storeImpl<uint16_t>( addr, value); }
    Expected<void> store32( uint32_t addr, uint32_t value) { return storeImpl<uint32_t>( addr, value); }

private:
    MemoryRegion* findRegion( uint32_t addr, uint32_t size);
    template<MemoryAccessType T> Expected<T> loadImpl( uint32_t addr, bool executable = false);
    template<MemoryAccessType T> Expected<void> storeImpl( uint32_t addr, T value);

private:
    std::vector<MemoryRegion> memory_regions_;

};

} // ! namespace mem
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_MEMORY_HH__
