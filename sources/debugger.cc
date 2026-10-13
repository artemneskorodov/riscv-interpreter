#include <string>
#include <iostream>
#include <istream>
#include <sstream>
#include <string_view>
#include <iomanip>
#include <optional>

#include "debugger.hh"
#include "disasm.hh"

namespace riscv
{

std::optional<uint32_t>
getUint32Value( std::istringstream& input)
{
    std::string n_str;
    if ( !(input >> n_str) )
    {
        return std::nullopt;
    }

    char* endptr = nullptr;
    uint64_t value = std::strtoull( n_str.c_str(), &endptr, 0);
    if ( value > UINT32_MAX )
    {
        return std::nullopt;
    }
    return static_cast<uint32_t>( value);
}

Debugger::Action
Debugger::prompt( CPUModel& cpu_model)
{
    auto bits = cpu_model.memory().loadExecutable( cpu_model.getPC());
    if ( bits.ok() )
    {
        std::cout << disasm::instrToStr( bits.value(), cpu_model.getPC()) << std::endl;
    }

    for ( ; ; )
    {
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
            return Action::Step;
        } else if ( command == "continue" )
        {
            return Action::Continue;
        } else if ( command == "quit" )
        {
            return Action::Quit;
        } else if ( command == "x" )
        {
            constexpr std::string_view kErrorMsg = "Usage: x <n>, where n is a number of "
                                                   "register (0-31)";
            auto n = getUint32Value( input);
            if ( !n.has_value() )
            {
                std::cerr << kErrorMsg << std::endl;
                continue;
            }

            std::cout << "x[" << std::dec << n.value() << "] = 0x"
                      << std::hex << std::setw( 10) << std::setfill( '0')
                      << cpu_model.getX( n.value()) << std::endl;
            continue;
        } else if ( command == "pc" )
        {
            std::cout << "pc = 0x" << std::hex << std::setw( 10) << std::setfill( '0')
                      << cpu_model.getPC() << std::endl;
            continue;
        } else if ( command == "memory" )
        {
            constexpr std::string_view kErrorMsg = "Usage: memory <base> <size>";
            auto base = getUint32Value( input);
            auto size = getUint32Value( input);
            if ( !base.has_value() || !size.has_value() )
            {
                std::cerr << kErrorMsg << std::endl;
                continue;
            }
            std::vector<std::uint8_t> data;
            data.resize( size.value());
            auto r = cpu_model.memory().copyFromGuest( base.value(), data.data(), size.value());
            if ( !r.ok() )
            {
                std::cerr << "Error while reading memory region: " << r.error().message << std::endl;
                continue;
            }

            std::string result = "";
            for ( uint32_t addr = base.value(); addr < base.value() + size.value(); addr += 8 )
            {
                result += std::format( "{:#010x}", addr);

                uint32_t end = std::min( size.value() + base.value() - addr, 8u);
                for ( uint32_t i = 0; i != end; ++i )
                {
                    result += std::format( " {:02x}", data[addr - base.value() + i]);
                }
                result += "\n";
            }
            std::cout << result;
            continue;
        } else
        {
            std::cerr << "Unknown command" << std::endl;
            continue;
        }
    }
}

} // ! namespace riscv
