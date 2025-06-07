#pragma once
#include <string_view>
#include <vector>
#include "Audio/Wav/WavSplitter.h"

class AudioConverter final
{
public:
    AudioConverter(std::vector<std::string_view> options) noexcept
    {
        m_Options = std::move(options);
    }

    bool parse() noexcept
    {
        if (m_Options.empty()) {
            return false;
        }

        m_SubCommand = toSubCommand(m_Options.front());

        if (m_SubCommand != SubCommand::Unknown) {
            m_Options.erase(m_Options.begin());
        }

        for (size_t i = 0; i < m_Options.size(); ++i) {
            auto& option = m_Options[i];
            switch (toOption(option)) {
            case AudioConverterOption::InputFile:
                m_InputFile = (i + 1 < m_Options.size()) ? m_Options[++i] : "";
                break;
            case AudioConverterOption::OutputFile:
                m_OutputFile = (i + 1 < m_Options.size()) ? m_Options[++i] : "";
                break;
            default:
                break;
            }
        }
        return m_SubCommand == SubCommand::Split && !m_InputFile.empty() && !m_OutputFile.empty();
    }

    bool execute() noexcept
    {
        switch (m_SubCommand) {
        case SubCommand::Split:
            return executeSplit();
        default:
            return false;
        }
    }

private:
    enum class SubCommand
    {
        Split,
        Unknown
    };

    SubCommand toSubCommand(std::string_view sub) const noexcept
    {
        if (sub == "split") {
            return SubCommand::Split;
        }
        return SubCommand::Unknown;
    }

    bool executeSplit() noexcept
    {
        if (m_InputFile.ends_with(".wav")) {
            using namespace RagiMagick2::Audio::Wav;
            WavSplitter splitter(m_InputFile, m_OutputFile);
            return splitter.execute();
        }

        return false;
    }

    enum class AudioConverterOption
    {
        InputFile,
        OutputFile,
        Unknown
    };

    AudioConverterOption toOption(std::string_view option) const noexcept
    {
        if (option == "--input-file" || option == "-i") {
            return AudioConverterOption::InputFile;
        }
        if (option == "--output-file" || option == "-o") {
            return AudioConverterOption::OutputFile;
        }
        return AudioConverterOption::Unknown;
    }

    std::vector<std::string_view> m_Options;
    std::string_view m_InputFile;
    std::string_view m_OutputFile;
    SubCommand m_SubCommand = SubCommand::Unknown;
};
