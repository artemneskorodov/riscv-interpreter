#include <iostream>

#include "instructions.hh"

int
main( int /*argc*/,
      const char* /*argv*/[])
{
    for ( auto elem : riscv::isa::kEncodings )
    {
        std::cout << elem.name << std::endl;
    }
    return 0;
}
