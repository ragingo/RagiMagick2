#pragma once
#include <array>
#include <cstdint>

namespace RagiMagick2::Audio::Wav
{
    enum class WavFormat : uint16_t
    {
        WAVE_FORMAT_UNKNOWN = 0x0000,
        WAVE_FORMAT_PCM = 0x0001,
        WAVE_FORMAT_EXTENSIBLE = 0xfffe
    };

    struct WavFileHeader
    {
        std::array<uint8_t, 4> fourcc{};
        uint32_t length;
        std::array<uint8_t, 4> format{};
    };

    struct FormatChunk
    {
        std::array<uint8_t, 4> fourcc{};
        uint32_t length;
        WavFormat format;
        uint16_t channels;
        uint32_t samplingFreq;
        uint32_t bytesPerSec;
        uint16_t blockSize;
        uint16_t bitsPerSample;
    };
}
