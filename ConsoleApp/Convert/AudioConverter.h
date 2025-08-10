#pragma once
#include <algorithm>
#include <charconv>
#include <optional>
#include <print>
#include <string_view>
#include <vector>
#include "Audio/Wav/WavSplitter.h"
#include "Audio/Wav/WavProcessor.h"
#include "Audio/Wav/Wav.h"

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
            m_SubCommand = SubCommand::Unknown;
            return false;
        }

        SubCommand explicitCommand = toSubCommand(m_Options.front());

        if (explicitCommand != SubCommand::Unknown) {
            m_SubCommand = explicitCommand;
            m_Options.erase(m_Options.begin());
        } else if (isOption(m_Options.front())) {
            m_SubCommand = SubCommand::Process;
        } else {
            m_SubCommand = SubCommand::Unknown;
            return false;
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
            case AudioConverterOption::Speed:
                if (i + 1 < m_Options.size()) {
                    m_Speed = parseDouble(m_Options[++i]);
                }
                break;
            case AudioConverterOption::Channels:
                if (i + 1 < m_Options.size()) {
                    m_Channels = m_Options[++i];
                }
                break;
            default:
                break;
            }
        }
        return !m_InputFile.empty() && !m_OutputFile.empty();
    }

    bool execute() noexcept
    {
        switch (m_SubCommand) {
        case SubCommand::Process:
            return executeProcess();
        case SubCommand::Split:
            return executeSplit();
        default:
            return false;
        }
    }

private:
    enum class SubCommand
    {
        Process,
        Split,
        Unknown
    };

    SubCommand toSubCommand(std::string_view sub) const noexcept
    {
        if (sub == "process") {
            return SubCommand::Process;
        }
        if (sub == "split") {
            return SubCommand::Split;
        }
        return SubCommand::Unknown;
    }

    enum class AudioConverterOption
    {
        InputFile,
        OutputFile,
        Speed,
        Channels,
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
        if (option == "--speed") {
            return AudioConverterOption::Speed;
        }
        if (option == "--channels") {
            return AudioConverterOption::Channels;
        }
        return AudioConverterOption::Unknown;
    }

    bool isOption(std::string_view sv) const noexcept {
        return sv.starts_with("--") || sv.starts_with("-");
    }

    std::optional<double> parseDouble(std::string_view sv) const noexcept {
        double value = 0.0;
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
        if (ec == std::errc{}) {
            return value;
        }
        return std::nullopt;
    }

    bool executeProcess() noexcept
    {
        RagiMagick2::Audio::Wav::WavProcessor processor(m_InputFile, m_OutputFile);

        if (m_Speed.has_value()) {
            processor.setSpeed(m_Speed.value());
        }

        if (m_Channels.has_value()) {
            if (m_Channels.value() == "mono") {
                processor.setChannels(RagiMagick2::Audio::Wav::Channel::MONO);
            } else if (m_Channels.value() == "stereo") {
                processor.setChannels(RagiMagick2::Audio::Wav::Channel::STEREO);
            }
        }

        if (!processor.execute()) {
            return false;
        }
        return true;
    }

    bool executeSplit() noexcept
    {
        if (m_InputFile.ends_with(".wav")) {
            RagiMagick2::Audio::Wav::WavSplitter splitter(m_InputFile, m_OutputFile);
            return splitter.execute();
        }
        return false;
    }

    std::vector<std::string_view> m_Options;
    std::string_view m_InputFile;
    std::string_view m_OutputFile;
    std::optional<double> m_Speed;
    std::optional<std::string_view> m_Channels;
    SubCommand m_SubCommand = SubCommand::Unknown;
};
