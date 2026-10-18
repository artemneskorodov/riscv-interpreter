#ifndef RVI_MACHINE_MEMORY_HH__
#define RVI_MACHINE_MEMORY_HH__

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>
#include <concepts>
#include <span>

#include "helpers/error.hh"
#include "isa/cause.hh"

namespace rvi
{
namespace machine
{

class MemoryPermissions
{
public:
    enum Permissions
    {
        EXECUTE = (1 << 0),
        WRITE   = (1 << 1),
        READ    = (1 << 2),
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
        uint64_t access_end = static_cast<uint64_t>( addr) + sz;
        uint64_t region_end = static_cast<uint64_t>( base) + size;
        return (addr >= base) && (access_end <= region_end);
    }

    bool
    overlaps( uint32_t addr,
              uint32_t sz) const
    {
        uint64_t this_begin = base;
        uint64_t this_end = this_begin + size;
        uint64_t other_begin = addr;
        uint64_t other_end = other_begin + sz;

        return (this_begin < other_end) && (other_begin < this_end);
    }
};

template<typename T>
concept MemoryAccessType = std::same_as<T, uint8_t>
                           || std::same_as<T, uint16_t>
                           || std::same_as<T, uint32_t>;

template<typename T>
using MemoryResult = Expected<T, isa::Trap>;

class Memory
{
public:
    InterpreterResult<void> allocate( uint32_t base,
                                      uint32_t size,
                                      MemoryPermissions permissions,
                                      const void* load,
                                      std::size_t load_size);

    template<MemoryAccessType T>
    MemoryResult<T>
    load( uint32_t addr)
    {
        return loadImpl<T>( addr);
    }

    MemoryResult<uint32_t>
    loadExecutable( uint32_t addr)
    {
        return loadImpl<uint32_t>( addr, true);
    }

    template<MemoryAccessType T>
    MemoryResult<void>
    store( uint32_t addr, T value)
    {
        return storeImpl<T>( addr, value);
    }

    InterpreterResult<void> copyFromGuest( uint32_t addr, void* dst, uint32_t size);
    InterpreterResult<void> copyToGuest( uint32_t addr, const void* src, uint32_t size);

    void loadTrusted( uint32_t addr, uint32_t size, void* data);
    void storeTrusted( uint32_t addr, uint32_t size, const void* data);

public:
    std::vector<std::pair<uint32_t, uint32_t>> getCommitedRanges() const;

private:
    MemoryRegion* findRegion( uint32_t addr, uint32_t size);

    MemoryRegion* findRegionContaining( uint32_t addr);

    template<MemoryAccessType T>
    MemoryResult<T> loadImpl( uint32_t addr, bool executable = false);

    template<MemoryAccessType T>
    MemoryResult<void> storeImpl( uint32_t addr, T value);

    InterpreterResult<void> validateMemoryRange( uint32_t addr,
                                                 uint32_t size,
                                                 MemoryPermissions permissions);

private:
    std::vector<MemoryRegion> memory_regions_;

};

} // ! namespace machine
} // ! namespace rvi

#endif // ! RVI_MACHINE_MEMORY_HH__
