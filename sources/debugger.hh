#ifndef RISCV_INTERPRETER_DEBUGGER_HH__
#define RISCV_INTERPRETER_DEBUGGER_HH__

#include <cstdint>
#include <variant>
#include <set>

#include "cpu_model.hh"

namespace riscv
{

class Debugger
{
public:
    std::optional<uint32_t> prompt( CPUModel& cpu_model);

private:
    std::set<uint32_t> breakpoints_;

};

} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_DEBUGGER_HH__
