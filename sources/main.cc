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

    std::string elf_filename( argv[1]);
    std::vector<std::string_view> arguments( argv + 2, argv + argc);

    //
    // Loading ELF file
    //
    {
        auto r = riscv::elf::loadElfFile( cpu_model.memory(), elf_filename);
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
        auto r = riscv::abi::prepareMainArguments( cpu_model, arguments);
        if ( !r.ok() )
        {
            RVI_CRITICAL( "Error while preparing main arguments: {}", r.error().message);
            return static_cast<int>( r.error().error);
        }
    }

    //
    // Running instructions
    //
    {
        for ( ; ; )
        {
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
}
