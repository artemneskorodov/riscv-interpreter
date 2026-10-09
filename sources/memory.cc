#include <format>

#include "memory.hh"

namespace riscv
{
namespace mem
{

Expected<void>
Memory::allocate( uint32_t base,
                  uint32_t size,
                  MemoryPermissions permissions,
                  const void* load,
                  std::size_t load_size)
{
    RVI_ASSERT( load_size <= size);
    for ( const MemoryRegion& region : memory_regions_ )
    {
        if ( region.overlaps( base, size) )
        {
            return Error( ErrorCode::MemoryRegionsOverlap,
                          std::format( "Memory regions overlap:\n"
                                       "Already created: [{}; {})\n",
                                       "Asked to allocate: [{}; {})\n",
                                       region.base, region.base + region.size,
                                       base, base + size));
        }
    }

    memory_regions_.emplace_back( base, size, permissions);
    if ( (load != nullptr) && (load_size != 0) )
    {
        std::memcpy( memory_regions_.back().data.get(), load, load_size);
    }
    return Expected<void>();
}

MemoryRegion*
Memory::findRegion( uint32_t addr,
                    uint32_t size)
{
    for ( MemoryRegion& region : memory_regions_ )
    {
        if ( region.contains( addr, size) )
        {
            return &region;
        }
    }
    return nullptr;
}

template<MemoryAccessType T>
Expected<T>
Memory::loadImpl( uint32_t addr,
                  bool executable)
{
    MemoryRegion* region = findRegion( addr, sizeof( T));
    if ( region == nullptr )
    {
        return Error( ErrorCode::InvalidMemoryAccess,
                      std::format( "Uncommited memory access: [{}; {})",
                                   addr, addr + sizeof( T)));
    }

    if ( !region->permissions.Read() )
    {
        return Error( ErrorCode::InvalidMemoryAccess,
                      std::format( "Load from memory, without read permissions: [{}; {})",
                                   addr, addr + sizeof( T)));
    }

    if ( executable && !region->permissions.Execute() )
    {
        return Error( ErrorCode::InvalidMemoryAccess,
                      std::format( "Load executable from memory, without execute permission: "
                                   "[{}; {})",
                                   addr, addr + sizeof( T)));
    }

    T value{};
    std::memcpy( &value, region->data.get() + addr, sizeof( value));
    return value;
}

template<MemoryAccessType T>
Expected<void>
Memory::storeImpl( uint32_t addr, T value)
{
    MemoryRegion* region = findRegion( addr, sizeof( T));
    if ( region == nullptr )
    {
        return Error( ErrorCode::InvalidMemoryAccess,
                      std::format( "Uncommited memory access: [{}; {})",
                                   addr, addr + sizeof( T)));
    }

    if ( !region->permissions.Write() )
    {
        return Error( ErrorCode::InvalidMemoryAccess,
                      std::format( "Store to memory, without write permissions: [{}; {})",
                                   addr, addr + sizeof( T)));
    }

    std::memcpy( region->data().get() + addr, &value, sizeof( value));
    return Expected<void>();
}

} // ! namespace mem
} // ! namespace riscv
