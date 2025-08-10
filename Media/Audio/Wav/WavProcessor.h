#pragma once

#include <string_view>
#include <optional>
#include <vector>
#include "Audio/Wav/Wav.h"

namespace RagiMagick2::Audio::Wav
{
    class WavProcessor final
    {
    public:
        WavProcessor(std::string_view inputFile, std::string_view outputFile) noexcept;
        void setSpeed(double speed) noexcept {
            m_Speed = speed;
        }
        void setChannels(Channel channels) noexcept {
            m_Channels = channels;
        }
        bool execute() noexcept;

    private:
        std::string_view m_InputFile;
        std::string_view m_OutputFile;
        double m_Speed;
        std::optional<Channel> m_Channels;

        void updateChannels(const FormatChunk& inputFormatChunk, FormatChunk& outputFormatChunk) noexcept;
        void updateSpeed(const FormatChunk& inputFormatChunk, FormatChunk& outputFormatChunk) noexcept;

        void convertChannels(
            const FormatChunk& inputFormatChunk,
            const FormatChunk& outputFormatChunk,
            const std::vector<uint8_t>& inBuf,
            std::vector<uint8_t>& outBuf,
            uint32_t sampleCount
        ) noexcept;

        template <typename T>
        void convertStereoToMono(const std::vector<uint8_t>& inBuf, std::vector<uint8_t>& outBuf, uint32_t sampleCount) noexcept;

        template <typename T>
        void convertMonoToStereo(const std::vector<uint8_t>& inBuf, std::vector<uint8_t>& outBuf, uint32_t sampleCount) noexcept;
    };
} // namespace RagiMagick2::Audio::Wav
