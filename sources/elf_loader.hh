#ifndef RISCV_INTERPRETER_ELF_LOADER_HH__
#define RISCV_INTERPRETER_ELF_LOADER_HH__

#include <filesystem>
#include <expected>
#include <cstdint>
#include <string_view>

#include "memory.hh"
#include "errors.hh"

namespace riscv
{
namespace elf
{

//
// Loads ELF file to memory.
// Returns entry point on success.
//
Expected<uint32_t> loadElfFile( mem::Memory& memory, const std::string_view& elf_filename);

} // ! namespace elf
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_ELF_LOADER_HH__
