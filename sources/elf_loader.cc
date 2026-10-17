#include <optional>

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
    bool need_allocation;
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
        RVI_WARN( "Skipping not PT_LOAD type segment: actual type is {}",
                  static_cast<int>( segment.get_type()));
        return Allocation( false, 0, 0, 0, 0);
    }

    uint32_t m_offset = segment.get_virtual_address();
    uint32_t m_size = segment.get_memory_size();
    uint32_t f_size = segment.get_file_size();

    uint32_t flags = segment.get_flags();

    return Allocation( true, m_offset, m_size, f_size, mem::MemoryPermissions( flags));
}

//
// Finds symbol "__global_pointer$" in symtab.
// We need this to implement part of code, expected to be in _start.
// Description of corresponding linker global-pointer relaxation:
// https://docs.riscv.org/reference/abi/v1.0/riscv-elf-linker-relaxation.html.
//
std::optional<uint32_t>
findGlobalPointer( const ELFIO::elfio& elf)
{
    for ( const std::unique_ptr<ELFIO::section>& section_ptr : elf.sections )
    {
        const ELFIO::section& section = *section_ptr;
        if ( section.get_type() != ELFIO::SHT_SYMTAB
             && section.get_type() != ELFIO::SHT_DYNSYM )
        {
            continue;
        }

        const ELFIO::const_symbol_section_accessor symbols( elf, &section);
        ELFIO::Elf64_Addr value = 0;
        ELFIO::Elf_Xword size = 0;
        unsigned char bind = 0;
        unsigned char type = 0;
        ELFIO::Elf_Half section_index = 0;
        unsigned char other = 0;

        if ( symbols.get_symbol( "__global_pointer$",
                                 value,
                                 size,
                                 bind,
                                 type,
                                 section_index,
                                 other) )
        {
            return static_cast<uint32_t>( value);
        }
    }

    return std::nullopt;
}

} // ! anonymous namespace

Expected<LoadedElf>
loadElfFile( mem::Memory& memory,
             const std::string_view& elf_filename)
{
    RVI_TRACE( "Loading ELF file: {}", elf_filename);

    ELFIO::elfio elf;
    if ( !elf.load( std::string( elf_filename)) )
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
        RVI_DEBUG( "Got entry point for ELF: {:#x}", entry);
    }

    //
    // Reading program headers
    //
    for ( const std::unique_ptr<ELFIO::segment>& segment_ptr : elf.segments )
    {
        const ELFIO::segment& segment = *segment_ptr;

        auto segment_r = parseProgramHeader( segment);
        if ( !segment_r.ok() )
        {
            return segment_r.error();
        }
        const Allocation& alloc = segment_r.value();

        if ( alloc.need_allocation )
        {
            const void* data_ptr = nullptr;
            if ( alloc.f_size != 0 )
            {
                data_ptr = segment.get_data();
            }
            RVI_DEBUG( "Allocating memory: [{:#x}:{:#x}) {}{}{}",
                       alloc.m_offset,
                       alloc.m_offset + alloc.m_size,
                       alloc.permissions.Read() ? "R" : " ",
                       alloc.permissions.Write() ? "W" : " ",
                       alloc.permissions.Execute() ? "X" : " ");
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
    }

    std::optional<uint32_t> global_pointer = findGlobalPointer( elf);
    if ( global_pointer.has_value() )
    {
        RVI_DEBUG( "Got global pointer for ELF: {:#x}", global_pointer.value());
    }

    return LoadedElf{ entry, global_pointer };
}

} // ! namespace elf
} // ! namespace riscv
