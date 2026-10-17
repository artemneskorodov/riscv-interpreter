#ifndef RVI_RUNTIME_PROCESS_HH__
#define RVI_RUNTIME_PROCESS_HH__

#include <cstdint>
#include <vector>
#include <string_view>

#include "machine/memory.hh"
#include "machine/cpu.hh"
#include "helpers/error.hh"

namespace rvi
{
namespace runtime
{

//
// Guest stack parameters.
//
constexpr uint32_t kDefaultStackSize = 1 * 1024 * 1024;
constexpr uint32_t kDefaultStackTop = 0xc0000000;

//
// Allocates guest stack in memory.
//
Expected<void> allocateGuestStack( machine::Memory& memory,
                                   uint32_t top = kDefaultStackTop,
                                   uint32_t size = kDefaultStackSize);

//
// Store program arguments to guest stack, creates main parameters and points sp.
//
Expected<void> prepareMainArguments( CPUModel& cpu_model,
                                     const std::vector<std::string_view>& args,
                                     uint32_t initial_sp = kDefaultStackTop);

} // ! namespace runtime
} // ! namespace rvi

#endif // ! RVI_RUNTIME_PROCESS_HH__
