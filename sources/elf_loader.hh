#ifndef RISCV_INTERPRETER_ELF_LOADER_HH__
#define RISCV_INTERPRETER_ELF_LOADER_HH__

#include <filesystem>
#include <expected>
#include <cstdint>
#include <optional>
#include <string_view>

#include "memory.hh"
#include "errors.hh"

namespace riscv
{
namespace elf
{

struct LoadedElf
{
    uint32_t entry;
    std::optional<uint32_t> global_pointer;
};

//
// Loads ELF file to memory.
// Returns the initial program state described by the ELF on success.
//
Expected<LoadedElf> loadElfFile( mem::Memory& memory, const std::string_view& elf_filename);

} // ! namespace elf
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_ELF_LOADER_HH__
