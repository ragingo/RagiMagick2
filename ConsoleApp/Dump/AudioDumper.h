#pragma once
#include <print>
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
        if (!parser.parse()) {
            return false;
        }

        if (parser.hasCue()) {
            const auto& cue = *parser.getCue();
            const auto& tracks = parser.getTracks();
            std::println("===== Cue Sheet =====");
            for (const auto& track : cue.tracks) {
                std::println("Track ID: {}, Title: {}, Performer: {}, Type: {}", track.id, track.title, track.performer, track.type);
                for (const auto& index : track.indices) {
                    std::println("  Index: {}, Minutes: {}, Seconds: {}, Frames: {}", index.index, index.minutes, index.seconds, index.frames);
                }
            }
            std::println();
            std::println("===== Raw Data =====");
            for (const auto& track : tracks) {
                std::println("Track ID: {}, Title: {}, Length: {}, Offset: {}", track.id, track.title, track.soundLength, track.soundOffset);
            }
        }

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
