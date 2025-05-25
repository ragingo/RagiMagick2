#pragma once
#include <array>
#include <cstdint>

namespace RagiMagick2::Audio::Wav
{
    struct WavFileHeader
    {
        std::array<uint8_t, 4> fourcc{};
        uint32_t length;
        std::array<uint8_t, 4> format{};
    };

    struct FormatChunk
    {
        uint32_t length;
        uint16_t format;
        uint16_t channels;
        uint32_t samplingFreq;
        uint32_t bytesPerSec;
        uint16_t blockSize;
        uint16_t bitsPerSample;
    };
}
