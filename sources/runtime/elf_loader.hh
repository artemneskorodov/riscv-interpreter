#ifndef RVI_RUNTIME_ELF_LOADER_HH__
#define RVI_RUNTIME_ELF_LOADER_HH__

#include <filesystem>
#include <expected>
#include <cstdint>
#include <optional>
#include <string_view>

#include "machine/memory.hh"
#include "helpers/error.hh"

namespace rvi
{
namespace runtime
{

//
// Information about loaded ELF file.
//
struct LoadedElf
{
    uint32_t entry;
    std::optional<uint32_t> global_pointer;
};

//
// Loads ELF file to memory.
// Returns the initial program state described by the ELF on success.
//
InterpreterResult<LoadedElf> loadElfFile( machine::Memory& memory,
                                          const std::string_view& elf_filename);

} // ! namespace runtime
} // ! namespace rvi

#endif // ! RVI_RUNTIME_ELF_LOADER_HH__
