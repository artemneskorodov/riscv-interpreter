#ifndef RISCV_INTERPRETER_DEBUGGER_HH__
#define RISCV_INTERPRETER_DEBUGGER_HH__

#include "cpu_model.hh"

namespace riscv
{

class Debugger
{
public:
    enum class Action
    {
        Step,
        Continue,
        Quit,
    };

    Action prompt( CPUModel& cpu_model);

};

} // ! namespace riscv

#endif // ! RISCV_INTERPRETER_DEBUGGER_HH__
