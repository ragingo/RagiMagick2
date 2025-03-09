#pragma once

#include <vector>
#include <print>
#include <string_view>
#include <type_traits>
#include "CommandLine/ConvertCommand.h"
#include "CommandLine/Options.h"

class CommandLine final
{
public:
    CommandLine(std::vector<std::string_view> options)
    {
        m_Options = std::move(options);
    }

    bool parse() noexcept
    {
        if (m_Options.empty()) {
            std::println("Usage: ConsoleApp <command> <subcommand> [options]");
            return false;
        }
        m_Command = toCommand(m_Options.front());
        m_Options.erase(m_Options.begin());
        return m_Command != Command::Unknown;
    }

    bool execute() noexcept
    {
        switch (m_Command) {
        case Command::Convert:
        {
            auto command = ConvertCommand(m_Options);
            if (!command.parse()) {
                return false;
            }
            if (!command.execute()) {
                return false;
            }
            return true;
        }
        case Command::Help:
            return true;
        default:
            return false;
        }
    }

private:
    Command toCommand(std::string_view command) const
    {
        if (command == "convert") {
            return Command::Convert;
        }
        if (command == "help") {
            return Command::Help;
        }
        return Command::Unknown;
    }

private:
    std::vector<std::string_view> m_Options;
    Command m_Command = Command::Unknown;
};
