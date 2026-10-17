#include <format>

#include "machine/memory.hh"

namespace rvi
{
namespace machine
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

Expected<void>
Memory::copyFromGuest( uint32_t addr,
                       void* dst,
                       uint32_t size)
{
    auto r = validateMemoryRange( addr, size, MemoryPermissions::READ);
    if ( !r.ok() )
    {
        return r.error();
    }

    uint32_t copied = 0;

    while ( copied < size )
    {
        MemoryRegion* region = findRegionContaining( addr + copied);
        RVI_ASSERT( region != nullptr);

        uint32_t region_offset = addr + copied - region->base;
        uint32_t piece_size = std::min<uint32_t>( region->size - region_offset,
                                                  size - copied);
        std::memcpy( static_cast<std::byte*>( dst) + copied,
                     region->data.get() + region_offset,
                     piece_size);
        copied += piece_size;
    }

    return Expected<void>();
}

Expected<void>
Memory::copyToGuest( uint32_t addr,
                     const void* src,
                     uint32_t size)
{
    auto r = validateMemoryRange( addr, size, MemoryPermissions::WRITE);
    if ( !r.ok() )
    {
        return r.error();
    }

    uint32_t copied = 0;

    while ( copied < size )
    {
        MemoryRegion* region = findRegionContaining( addr + copied);
        RVI_ASSERT( region != nullptr);

        uint32_t region_offset = addr + copied - region->base;
        uint32_t piece_size = std::min<uint32_t>( region->size - region_offset,
                                                  size - copied);
        std::memcpy( region->data.get() + region_offset,
                     static_cast<const std::byte*>( src) + copied,
                     piece_size);
        copied += piece_size;
    }

    return Expected<void>();
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

MemoryRegion*
Memory::findRegionContaining( uint32_t addr)
{
    for ( MemoryRegion& region : memory_regions_ )
    {
        if ( (addr >= region.base) && (addr < region.base + region.size) )
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

Expected<void>
Memory::validateMemoryRange( uint32_t addr,
                             uint32_t size,
                             MemoryPermissions permissions)
{
    uint32_t begin = addr;
    if ( static_cast<uint64_t>( size) + static_cast<uint64_t>( begin) > UINT32_MAX )
    {
        return Error( ErrorCode::InvalidMemoryAccess,
                      std::format( "Memory exceeds RV32 address space: addr={:#x}, size={:#x}",
                                   addr, size));
    }
    uint32_t end = addr + size;
    if ( begin == end )
    {
        return Expected<void>();
    }

    while ( begin < end )
    {
        const MemoryRegion* region = findRegionContaining( begin);
        if ( region == nullptr )
        {
            return Error( ErrorCode::InvalidMemoryAccess,
                          std::format( "Access to uncommitted memory: [{:#x}, {:#x})",
                                       addr, addr + size));
        }

        begin = std::min( end, region->base + region->size);

        if ( permissions.Read() && !region->permissions.Read() )
        {
            return Error( ErrorCode::InvalidMemoryAccess,
                          std::format( "Read from memory without permissions: [{:#x}, {:#x})",
                                       addr, addr + size));
        }
        if ( permissions.Write() && !region->permissions.Write() )
        {
            return Error( ErrorCode::InvalidMemoryAccess,
                          std::format( "Write to memory without permissions: [{:#x}, {:#x})",
                                       addr, addr + size));
        }
        if ( permissions.Execute() && !region->permissions.Execute() )
        {
            return Error( ErrorCode::InvalidMemoryAccess,
                          std::format( "Access to executable memory without permissions: "
                                       "[{:#x}, {:#x})",
                                       addr, addr + size));
        }
    }

    return Expected<void>();
}

} // ! namespace machine
} // ! namespace rvi
