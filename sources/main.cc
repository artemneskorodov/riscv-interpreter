#include <iostream>
#include <cstdlib>
#include <format>
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

namespace
{

int
reportInterpreterError( std::string_view context,
                        const rvi::InterpreterError& error)
{
    std::cerr << std::format( "Interpreter error {}: {}\n", context, error.message);
    return static_cast<int>( error.code);
}

int
reportGuestTrap( const rvi::interpreter::UnhandledTrap& trapped)
{
    std::cerr << std::format( "Guest trap: {} at pc={:#x}, value={:#x}\n",
                              rvi::isa::causeToStr( trapped.trap.cause),
                              trapped.pc,
                              trapped.trap.value);
    return EXIT_FAILURE;
}

} // ! anonymous namespace

int
main( int argc,
      const char* argv[])
{
    RVI_INIT_SPDLOG_LEVEL;

    rvi::CPUModel cpu_model{};
    if ( argc < 2 )
    {
        std::cerr << "Interpreter error: no ELF file specified\n";
        return static_cast<int>( rvi::InterpreterErrorCode::NoElfFileSpecified);
    }
    std::vector<std::string_view> args( argv + 1, argv + argc);

    //
    // Loading ELF file
    //
    {
        auto r = rvi::runtime::loadElfFile( cpu_model.memory(), args[0]);
        if ( !r.ok() )
        {
            return reportInterpreterError( "while loading ELF file", r.error());
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
            return reportInterpreterError( "while allocating guest stack", r.error());
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
            return reportInterpreterError( "while preparing main arguments", r.error());
        }
    }

    //
    // Running instructions
    //
    rvi::tools::Debugger debugger{};

    for ( ; ; )
    {
        // Calling debugger prompt before instruction execution.
        if ( debugger.enabled() && debugger.needPrompt( cpu_model) )
        {
            bool exit = debugger.prompt( cpu_model);
            if ( exit )
            {
                return 0;
            }
        }

        // Executing instruction.
        auto r = rvi::interpreter::runSingleInstruction( cpu_model);
        if ( !r.ok() )
        {
            return reportInterpreterError( "while running instruction", r.error());
        }

        if ( std::holds_alternative<rvi::interpreter::UnhandledTrap>( r.value()) )
        {
            const auto& trapped = std::get<rvi::interpreter::UnhandledTrap>( r.value());
            return reportGuestTrap( trapped);
        } else if ( std::holds_alternative<rvi::interpreter::Exit>( r.value()) )
        {
            int status = std::get<rvi::interpreter::Exit>( r.value()).status;
            RVI_DEBUG( "Exiting from main with status = {}", status);
            return status;
        }
    }
}
