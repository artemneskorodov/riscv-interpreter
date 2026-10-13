#include <string>
#include <iostream>
#include <istream>
#include <sstream>

#include "debugger.hh"
#include "disasm.hh"

namespace riscv
{

Debugger::Action
Debugger::prompt( CPUModel& cpu_model)
{
    for ( ; ; )
    {
        auto bits = cpu_model.memory().loadExecutable( cpu_model.getPC());
        if ( bits.ok() )
        {
            std::cout << disasm::instrToStr( bits.value(), cpu_model.getPC()) << std::endl;
        }
        std::cout << "> ";

        std::string line;
        if ( !std::getline( std::cin, line) )
        {
            return Action::Quit;
        }

        std::istringstream input( line);

        std::string command;
        input >> command;
        if ( command.empty() )
        {
            continue ;
        } else if ( command == "step" )
        {
        } else if ( command == "continue" )
        {
        } else if ( command == "quit" )
        {
        } else if ( command == "break" )
        {
            std::string addr_string;
            if ( !(input >> addr_string) )
            {
                std::cerr << "Usage: break <addr>" << std::endl;
                continue;
            }

            char* endptr = nullptr;
            uint64_t addr_value = std::strtoul( addr_string.c_str(), &endptr, 0);
            if ( endptr != addr_string.c_str() + addr_string.length() )
            {
                std::cerr << "Usage: break <addr>, where <addr> is integer value";
            }
            if ( addr_value > UINT32_MAX )
            {
                std::cerr << addr_string << " is greater than UINT32_MAX = " << UINT32_MAX << std::endl;
            }

            breakpoints_.emplace( static_cast<uint32_t>( addr_value));
        }
    }
}

} // ! namespace riscv
