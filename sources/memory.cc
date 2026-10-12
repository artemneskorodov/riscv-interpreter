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

void
Memory::loadTrusted( uint32_t addr,
                     uint32_t size,
                     void* data)
{
    MemoryRegion* region = findRegion( addr, size);
    RVI_ASSERT( region != nullptr);
    RVI_ASSERT( (region->permissions.Read()) != 0);

    std::memcpy( data, region->data.get() + addr - region->base, size);
}

void
Memory::storeTrusted( uint32_t addr,
                      uint32_t size,
                      const void* data)
{
    MemoryRegion* region = findRegion( addr, size);
    RVI_ASSERT( region != nullptr);
    RVI_ASSERT( (region->permissions.Write()) != 0);

    std::memcpy( region->data.get() + addr - region->base, data, size);
}

std::vector<std::pair<uint32_t, uint32_t>>
Memory::getCommitedRanges() const
{
    std::vector<std::pair<uint32_t, uint32_t>> ranges{};
    ranges.reserve( memory_regions_.size());
    for ( const MemoryRegion& region : memory_regions_ )
    {
        ranges.emplace_back( region.base, region.base + region.size);
    }
    return ranges;
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
                      std::format( "Uncommited memory access: [{:#x}; {:#x})",
                                   addr, addr + sizeof( T)));
    }

    if ( !region->permissions.Read() )
    {
        return Error( ErrorCode::InvalidMemoryAccess,
                      std::format( "Load from memory, without read permissions: [{:#x}; {:#x})",
                                   addr, addr + sizeof( T)));
    }

    if ( executable && !region->permissions.Execute() )
    {
        return Error( ErrorCode::InvalidMemoryAccess,
                      std::format( "Load executable from memory, without execute permission: "
                                   "[{:#x}; {:#x})",
                                   addr, addr + sizeof( T)));
    }

    T value{};
    std::memcpy( &value, region->data.get() + addr - region->base, sizeof( value));
    return value;
}

template Expected<uint8_t> Memory::loadImpl<uint8_t>( uint32_t , bool );
template Expected<uint16_t> Memory::loadImpl<uint16_t>( uint32_t , bool );
template Expected<uint32_t> Memory::loadImpl<uint32_t>( uint32_t , bool );

template<MemoryAccessType T>
Expected<void>
Memory::storeImpl( uint32_t addr, T value)
{
    MemoryRegion* region = findRegion( addr, sizeof( T));
    if ( region == nullptr )
    {
        return Error( ErrorCode::InvalidMemoryAccess,
                      std::format( "Uncommited memory access: [{:#x}; {:#x})",
                                   addr, addr + sizeof( T)));
    }

    if ( !region->permissions.Write() )
    {
        return Error( ErrorCode::InvalidMemoryAccess,
                      std::format( "Store to memory, without write permissions: [{:#x}; {:#x})",
                                   addr, addr + sizeof( T)));
    }

    std::memcpy( region->data.get() + addr - region->base, &value, sizeof( value));
    return Expected<void>();
}

template Expected<void> Memory::storeImpl<uint8_t>( uint32_t , uint8_t );
template Expected<void> Memory::storeImpl<uint16_t>( uint32_t , uint16_t );
template Expected<void> Memory::storeImpl<uint32_t>( uint32_t , uint32_t );

} // ! namespace mem
} // ! namespace riscv
