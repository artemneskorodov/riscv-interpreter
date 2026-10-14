#include <iostream>
#include <vector>
#include <string>
#include <string_view>

#include "instructions.hh"
#include "cpu_model.hh"
#include "debug.hh"
#include "elf_loader.hh"
#include "errors.hh"
#include "abi.hh"
#include "debugger.hh"

int
main( int argc,
      const char* argv[])
{
    RVI_INIT_SPDLOG_LEVEL;

    riscv::CPUModel cpu_model{};
    if ( argc < 2 )
    {
        RVI_CRITICAL( "No ELF file specified");
        return static_cast<int>( riscv::ErrorCode::NoElfFileSpecified);
    }

    std::vector<std::string_view> args( argv + 1, argv + argc);

    //
    // Loading ELF file
    //
    {
        auto r = riscv::elf::loadElfFile( cpu_model.memory(), args[0]);
        if ( !r.ok() )
        {
            RVI_CRITICAL( "Error while loading ELF file: {}", r.error().message);
            return static_cast<int>( r.error().error);
        }
        cpu_model.setPC(r.value());
    }

    //
    // Allocating guest stack
    //
    {
        auto r = riscv::abi::allocateGuestStack( cpu_model.memory());
        if ( !r.ok() )
        {
            RVI_CRITICAL( "Error while allocating guest stack: {}", r.error().message);
            return static_cast<int>( r.error().error);
        }
    }

    RVI_DEBUG( "Commited memory ranges: {}",
               riscv::debug::memoryRangesToStr( cpu_model.memory().getCommitedRanges()));

    //
    // Loading console arguments
    //
    {
        auto r = riscv::abi::prepareMainArguments( cpu_model, args);
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
    riscv::Debugger debugger{};
    bool call_debugger = true;
#endif // ! RVI_DEBUGGER_MODE_ENABLED

    for ( ; ; )
    {

#if RVI_DEBUGGER_MODE_ENABLED
        if ( call_debugger )
        {
            riscv::Debugger::Action action = debugger.prompt( cpu_model);

            switch ( action )
            {
                case riscv::Debugger::Action::Step:     break;
                case riscv::Debugger::Action::Continue: call_debugger = false; break;
                case riscv::Debugger::Action::Quit:     return 0;
            }
        }
#endif // ! RVI_DEBUGGER_MODE_ENABLED

        auto r = riscv::insn::runSingleInstruction( cpu_model);
        if ( !r.ok() )
        {
            RVI_CRITICAL( "Error while running instruction: {}", r.error().message);
            return static_cast<int>( r.error().error);
        }
        if ( std::holds_alternative<riscv::insn::Exit>( r.value()) )
        {
            int status = std::get<riscv::insn::Exit>( r.value()).status;
            RVI_DEBUG( "Exiting from main with status = {}", status);
            return status;
        }
    }
}
