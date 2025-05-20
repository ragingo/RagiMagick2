#pragma once

#include <type_traits>
#include <string_view>
#include <vector>
#include "Dump/AudioDumper.h"

class ShowCommand final
{
public:
    ShowCommand(std::vector<std::string_view> options) noexcept
    {
        m_Options = std::move(options);
    }

    bool parse() noexcept
    {
        if (m_Options.empty()) {
            return false;
        }
        m_SubCommand = toSubCommand(m_Options.front());
        m_Options.erase(m_Options.begin());
        return m_SubCommand != SubCommand::Unknown;
    }

    bool execute() noexcept
    {
        switch (m_SubCommand) {
        case SubCommand::Audio:
        {
            auto dumper = AudioDumper(m_Options);
            if (!dumper.parse()) {
                return false;
            }
            return dumper.execute();
        }
        case SubCommand::Help:
            return true;
        default:
            return false;
        }
    }

private:
    enum class SubCommand
    {
        Audio,
        Help,
        Unknown
    };

    SubCommand toSubCommand(std::string_view subCommand) const
    {
        if (subCommand == "audio") {
            return SubCommand::Audio;
        }
        if (subCommand == "help") {
            return SubCommand::Help;
        }
        return SubCommand::Unknown;
    }

private:
    std::vector<std::string_view> m_Options;
    SubCommand m_SubCommand = SubCommand::Unknown;
};
