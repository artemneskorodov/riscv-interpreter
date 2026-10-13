#ifndef RISCV_INTERPRETER_MEMORY_HH__
#define RISCV_INTERPRETER_MEMORY_HH__

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>
#include <concepts>
#include <span>

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
              uint32_t sz) const
    {
        return (addr >= base) && (addr + sz <= base + size);
    }

    bool
    overlaps( uint32_t addr,
              uint32_t sz) const
    {
        uint32_t this_begin = base;
        uint32_t this_end = this_begin + size;
        uint32_t other_begin = addr;
        uint32_t other_end = other_begin + sz;

        return (this_begin < other_end) && (other_begin < this_end);
    }
};

template<typename T>
concept MemoryAccessType = std::same_as<T, uint8_t>
                           || std::same_as<T, uint16_t>
                           || std::same_as<T, uint32_t>;

class Memory
{
public:
    Expected<void> allocate( uint32_t base,
                             uint32_t size,
                             MemoryPermissions permissions,
                             const void* load,
                             std::size_t load_size);

    template<MemoryAccessType T>
    Expected<T>
    load( uint32_t addr)
    {
        return loadImpl<T>( addr);
    }

    Expected<uint32_t> loadExecutable( uint32_t addr) { return loadImpl<uint32_t>( addr, true); }

    template<MemoryAccessType T>
    Expected<void>
    store( uint32_t addr, T value)
    {
        return storeImpl<T>( addr, value);
    }

    Expected<void> copyFromGuest( uint32_t addr, void* dst, uint32_t size);
    Expected<void> copyToGuest( uint32_t addr, const void* src, uint32_t size);

    void loadTrusted( uint32_t addr, uint32_t size, void* data);
    void storeTrusted( uint32_t addr, uint32_t size, const void* data);

public:
    std::vector<std::pair<uint32_t, uint32_t>> getCommitedRanges() const;

private:
    MemoryRegion* findRegion( uint32_t addr, uint32_t size);
    MemoryRegion* findRegionContaining( uint32_t addr);
    template<MemoryAccessType T> Expected<T> loadImpl( uint32_t addr, bool executable = false);
    template<MemoryAccessType T> Expected<void> storeImpl( uint32_t addr, T value);
    Expected<void> validateMemoryRange( uint32_t addr,
                                        uint32_t size,
                                        MemoryPermissions permissions);

private:
    std::vector<MemoryRegion> memory_regions_;

};

} // ! namespace mem
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_MEMORY_HH__
