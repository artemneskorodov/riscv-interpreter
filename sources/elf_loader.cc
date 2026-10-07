#include "elfio/elfio.hpp"

#include "elf_loader.hh"

namespace riscv
{
namespace elf
{

namespace
{

std::optional<ElfErrorCode>
validateElfHeader( const ELFIO::elfio& elf)
{
    if ( elf.get_class() != ELFIO::ELFCLASS32 )
    {
        return ElfErrorCode::UnexpectedElfClass;
    }

    if ( elf.get_encoding() != ELFIO::ELFDATA2LSB )
    {
        return ElfErrorCode::UnexpectedEncoding;
    }

    if ( elf.get_machine() != ELFIO::EM_RISCV )
    {
        return ElfErrorCode::UnexpectedMachine;
    }

    if ( elf.get_type() != ELFIO::ET_EXEC )
    {
        return ElfErrorCode::UnsupportedElfType;
    }

    if ( elf.get_version() != ELFIO::EV_CURRENT )
    {
        return ElfErrorCode::UnexpectedElfVersion;
    }

    return std::nullopt;
}

struct Allocation
{
    uint32_t m_offset;
    uint32_t m_size;
    uint32_t f_size;
};

std::expected<Allocation, ElfErrorCode>
parseProgramHeader( const ELFIO::segment& segment)
{
    if ( segment.get_type() != ELFIO::PT_LOAD )
    {
        return std::unexpected( ElfErrorCode::UnsupportedProgramHeaderType);
    }

    uint32_t m_offset = segment.get_virtual_address();
    uint32_t m_size = segment.get_memory_size();
    uint32_t f_size = segment.get_file_size();

    return Allocation( m_offset, m_size, f_size);
}

} // ! anonymous namespace

std::expected<void, ElfError>
loadElfFile( mem::Memory& memory,
             const std::string& elf_filename)
{
    ELFIO::elfio elf;
    if ( !elf.load( elf_filename) )
    {
        return std::unexpected( ElfError( ElfErrorCode::NoSuchFile, elf_filename));
    }

    //
    // Reading ELF header
    //
    uint32_t entry;
    {
        std::optional<ElfErrorCode> header_error = validateElfHeader( elf);
        if ( header_error.has_value() )
        {
            return std::unexpected( ElfError( *header_error, elf_filename));
        }
        entry = elf.get_entry();
    }

    //
    // Reading program headers
    //
    for ( const std::unique_ptr<ELFIO::segment>& segment_ptr : elf.segments )
    {
        const ELFIO::segment& segment = *segment_ptr;
        ELFIO::Elf_Word p_type = segment.get_type();

        std::expected<Allocation, ElfErrorCode> segment_error = parseProgramHeader( segment);
        if ( !segment_error.has_value() )
        {
            return std::unexpected( ElfError( segment_error.error(), elf_filename));
        }

        Allocation allocation = *segment_error;
        memory.allocate( allocation.m_offset, allocation.m_size);
        if ( allocation.f_size != 0 )
        {
            memory.store( allocation.m_offset, allocation.f_size, segment.get_data());
        }
    }
}

} // ! namespace elf
} // ! namespace riscv
