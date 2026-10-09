#include <iostream>
#include <vector>
#include <string>

#include "instructions.hh"
#include "cpu_model.hh"
#include "debug.hh"
#include "elf_loader.hh"

int
main( int argc,
      const char* argv[])
{
    riscv::CPUModel cpu_model{};
    std::vector<std::string> arguments( argv + 1, argv + argc);

    //
    // Loading ELF file
    //
    {
        auto r = riscv::elf::loadElfFile( cpu_model.memory(), arguments[0]);
        if ( !r.ok() )
        {
            RVI_CRITICAL( "Error while loading ELF file: {}", r.error().message);
            return static_cast<int>( r.error().error);
        }
    }

    //
    // Loading console arguments
    //
    {
        // TODO
    }

    //
    // Running instructions
    //
    {
        // TODO
    }
}
