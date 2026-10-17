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
    //
    // Returns true if exiting.
    //
    bool prompt( CPUModel& cpu_model);

    constexpr static bool
    enabled()
    {
        #if RVI_DEBUGGER_MODE_ENABLED
        return true;
        #else
        return false;
        #endif // RVI_DEBUGGER_MODE_ENABLED
    }

    constexpr bool
    needPrompt( CPUModel& c) const
    {
        if constexpr ( !enabled() )
        {
            return false;
        }

        if ( continue_enabled_ )
        {
            return false;
        }

        return true;
    }

private:
    bool continue_enabled_ = false;

};

} // ! namespace tools
} // ! namespace rvi

#endif // ! RVI_TOOLS_DEBUGGER_HH__
