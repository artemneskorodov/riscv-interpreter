#include <iostream>
#include <vector>
#include <string>
#include <string_view>

#include "interpreter/interpreter.hh"
#include "isa/registers.hh"
#include "machine/cpu.hh"
#include "helpers/diagnostics.hh"
#include "runtime/elf_loader.hh"
#include "helpers/error.hh"
#include "runtime/process.hh"
#include "tools/debugger.hh"

int
main( int argc,
      const char* argv[])
{
    RVI_INIT_SPDLOG_LEVEL;

    rvi::CPUModel cpu_model{};
    if ( argc < 2 )
    {
        RVI_CRITICAL( "No ELF file specified");
        return static_cast<int>( rvi::ErrorCode::NoElfFileSpecified);
    }

    std::vector<std::string_view> args( argv + 1, argv + argc);

    //
    // Loading ELF file
    //
    {
        auto r = rvi::runtime::loadElfFile( cpu_model.memory(), args[0]);
        if ( !r.ok() )
        {
            RVI_CRITICAL( "Error while loading ELF file: {}", r.error().message);
            return static_cast<int>( r.error().error);
        }
        const rvi::runtime::LoadedElf& loaded_elf = r.value();
        cpu_model.setPC( loaded_elf.entry);
        if ( loaded_elf.global_pointer.has_value() )
        {
            cpu_model.setX( rvi::isa::RegisterName::gp,
                            loaded_elf.global_pointer.value());
        }
    }

    //
    // Allocating guest stack
    //
    {
        auto r = rvi::runtime::allocateGuestStack( cpu_model.memory());
        if ( !r.ok() )
        {
            RVI_CRITICAL( "Error while allocating guest stack: {}", r.error().message);
            return static_cast<int>( r.error().error);
        }
    }

    RVI_DEBUG( "Commited memory ranges: {}",
               rvi::diagnostics::memoryRangesToStr( cpu_model.memory().getCommitedRanges()));

    //
    // Loading console arguments
    //
    {
        auto r = rvi::runtime::prepareMainArguments( cpu_model, args);
        if ( !r.ok() )
        {
            RVI_CRITICAL( "Error while preparing main arguments: {}", r.error().message);
            return static_cast<int>( r.error().error);
        }
    }

    //
    // Running instructions
    //

#if RVI_DEBUGGER_MODE_ENABLED
    rvi::tools::Debugger debugger{};
    bool call_debugger = true;
#endif // ! RVI_DEBUGGER_MODE_ENABLED

    for ( ; ; )
    {

#if RVI_DEBUGGER_MODE_ENABLED
        if ( call_debugger )
        {
            rvi::tools::Debugger::Action action = debugger.prompt( cpu_model);

            switch ( action )
            {
                case rvi::tools::Debugger::Action::Step:     break;
                case rvi::tools::Debugger::Action::Continue: call_debugger = false; break;
                case rvi::tools::Debugger::Action::Quit:     return 0;
            }
        }
#endif // ! RVI_DEBUGGER_MODE_ENABLED

        auto r = rvi::interpreter::runSingleInstruction( cpu_model);
        if ( !r.ok() )
        {
            RVI_CRITICAL( "Error while running instruction: {}", r.error().message);
            return static_cast<int>( r.error().error);
        }
        if ( std::holds_alternative<rvi::interpreter::Exit>( r.value()) )
        {
            int status = std::get<rvi::interpreter::Exit>( r.value()).status;
            RVI_DEBUG( "Exiting from main with status = {}", status);
            return status;
        }
    }
}
