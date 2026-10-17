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

    constexpr bool
    enabled() const
    {
        #if RVI_DEBUGGER_MODE_ENABLED
        return true;
        #else
        return false;
        #endif // RVI_DEBUGGER_MODE_ENABLED
    }
};

} // ! namespace tools
} // ! namespace rvi

#endif // ! RVI_TOOLS_DEBUGGER_HH__
