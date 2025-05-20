#pragma once
#include <string_view>
#include <vector>
#include "CommandLine/Options.h"
#include "Audio/Wav/WavParser.h"

class AudioDumper final
{
public:
    AudioDumper(std::vector<std::string_view> options)
    {
        m_Options = std::move(options);
    }

    bool parse() noexcept
    {
        for (size_t i = 0; i < m_Options.size(); ++i) {
            auto& option = m_Options[i];
            switch (toOption(option)) {
            case AudioShowOption::InputFile:
                m_InputFile = (i + 1 < m_Options.size()) ? m_Options[++i] : "";
                break;
            default:
                break;
            }
        }

        return !m_InputFile.empty();
    }

    bool execute()
    {
        using namespace RagiMagick2::Audio::Wav;

        WavParser parser(m_InputFile);
        parser.parse();

        return true;
    }

private:
    AudioShowOption toOption(std::string_view option) const noexcept
    {
        using enum AudioShowOption;
        if (option == "--input-file" || option == "-i") {
            return InputFile;
        }
        return Unknown;
    }

    std::vector<std::string_view> m_Options;
    std::string_view m_InputFile;
};
