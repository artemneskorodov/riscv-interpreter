#ifndef RVI_TOOLS_DEBUGGER_HH__
#define RVI_TOOLS_DEBUGGER_HH__

#include "machine/cpu.hh"

namespace rvi
{
namespace tools
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

} // ! namespace tools
} // ! namespace rvi

#endif // ! RVI_TOOLS_DEBUGGER_HH__
