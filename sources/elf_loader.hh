#ifndef RISCV_INTERPRETER_ELF_LOADER_HH__
#define RISCV_INTERPRETER_ELF_LOADER_HH__

#include <filesystem>
#include <expected>

#include "memory.hh"

namespace riscv
{
namespace elf
{

enum class ElfErrorCode
{
    NoSuchFile,
    UnexpectedElfClass,
    UnexpectedEncoding,
    UnexpectedMachine,
    UnsupportedElfType,
    UnexpectedElfVersion,
    UnsupportedProgramHeaderType,
};

struct ElfError
{
    ElfErrorCode error;
    std::filesystem::path path;
};

std::expected<void, ElfError> loadElfFile( mem::Memory& memory, const std::string& elf_filename);

} // ! namespace elf
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_ELF_LOADER_HH__
