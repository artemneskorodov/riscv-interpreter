#ifndef RVI_RUNTIME_SYSCALLS_HH__
#define RVI_RUNTIME_SYSCALLS_HH__

#include "interpreter/interpreter.hh"
#include "machine/cpu.hh"
#include "helpers/error.hh"

namespace rvi
{
namespace runtime
{

Expected<interpreter::RunAction> handleEnvironmentCall( CPUModel& cpu_model);

} // ! namespace runtime
} // ! namespace rvi

#endif // ! RVI_RUNTIME_SYSCALLS_HH__
