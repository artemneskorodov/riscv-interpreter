#include "elfio/elfio.hpp"

#include "elf_loader.hh"
#include "errors.hh"
#include "debug.hh"

namespace riscv
{
namespace elf
{

namespace
{

Expected<uint32_t>
parseElfHeader( const ELFIO::elfio& elf)
{
    if ( elf.get_class() != ELFIO::ELFCLASS32 )
    {
        return Error( ErrorCode::ElfClassUnexpected, "Expected only 32-bit ELFs");
    }
    if ( elf.get_encoding() != ELFIO::ELFDATA2LSB )
    {
        return Error( ErrorCode::ElfEncodingUnexpected, "Expected only LSB encoding");
    }
    if ( elf.get_machine() != ELFIO::EM_RISCV )
    {
        return Error( ErrorCode::ElfMachineUnexpected,
                      std::format( "Unexpected machine: {}, only EM_RISCV is supported",
                                   elf.get_machine()));
    }
    if ( elf.get_type() != ELFIO::ET_EXEC )
    {
        return Error( ErrorCode::ElfTypeUnexpected,
                      "Unexpected ELF type: only ET_EXEC is supported");
    }
    if ( elf.get_version() != ELFIO::EV_CURRENT )
    {
        return Error( ErrorCode::ElfVersionUnexpected,
                      "Only EV_CURRENT ELF version is supported");
    }

    return elf.get_entry();
}

struct Allocation
{
    uint32_t m_offset;
    uint32_t m_size;
    uint32_t f_size;
    mem::MemoryPermissions permissions;
};

Expected<Allocation>
parseProgramHeader( const ELFIO::segment& segment)
{
    if ( segment.get_type() != ELFIO::PT_LOAD )
    {
        return Error( ErrorCode::ElfSegmentTypeUnexpected,
                      "Expected only PT_LOAD segments");
    }

    uint32_t m_offset = segment.get_virtual_address();
    uint32_t m_size = segment.get_memory_size();
    uint32_t f_size = segment.get_file_size();

    uint32_t flags = segment.get_flags();

    return Allocation( m_offset, m_size, f_size, mem::MemoryPermissions( flags));
}

} // ! anonymous namespace

Expected<void>
loadElfFile( mem::Memory& memory,
             const std::string& elf_filename)
{
    ELFIO::elfio elf;
    if ( !elf.load( elf_filename) )
    {
        return Error( ErrorCode::ElfOpenFailed,
                      std::format( "Error while opening ELF file {}: {}",
                      elf_filename,
                      std::strerror( errno)));
    }

    //
    // Reading ELF header
    //
    uint32_t entry;
    {
        auto entry_r = parseElfHeader( elf);
        if ( !entry_r.ok() )
        {
            return entry_r.error();
        }
        entry = entry_r.value();
    }

    //
    // Reading program headers
    //
    for ( const std::unique_ptr<ELFIO::segment>& segment_ptr : elf.segments )
    {
        const ELFIO::segment& segment = *segment_ptr;
        ELFIO::Elf_Word p_type = segment.get_type();

        auto segment_r = parseProgramHeader( segment);
        if ( !segment_r.ok() )
        {
            return segment_r.error();
        }
        const Allocation& alloc = segment_r.value();

        const void* data_ptr = nullptr;
        if ( alloc.f_size != 0 )
        {
            data_ptr = segment.get_data();
        }
        auto alloc_r = memory.allocate( alloc.m_offset,
                                        alloc.m_size,
                                        alloc.permissions,
                                        data_ptr,
                                        alloc.f_size);
        if ( !alloc_r.ok() )
        {
            return alloc_r.error();
        }
    }

    return Expected<void>();
}

} // ! namespace elf
} // ! namespace riscv
