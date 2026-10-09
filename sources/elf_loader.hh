#ifndef RISCV_INTERPRETER_ELF_LOADER_HH__
#define RISCV_INTERPRETER_ELF_LOADER_HH__

#include <filesystem>
#include <expected>

#include "memory.hh"
#include "errors.hh"

namespace riscv
{
namespace elf
{

Expected<void> loadElfFile( mem::Memory& memory, const std::string& elf_filename);

} // ! namespace elf
} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_ELF_LOADER_HH__
