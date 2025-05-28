#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <ios>

namespace RagiMagick2::Audio::Wav
{
    enum class WavFormat : uint16_t
    {
        WAVE_FORMAT_UNKNOWN = 0x0000,
        WAVE_FORMAT_PCM = 0x0001,
        WAVE_FORMAT_EXTENSIBLE = 0xfffe
    };

    enum class ChunkID : uint32_t
    {
        UNKNOWN = 0,
        RIFF = 0x46464952, // 'RIFF' -> 'FFIR'
        FMT  = 0x20746d66, // 'fmt ' -> ' tmf'
        DATA = 0x61746164  // 'data' -> 'atad'
    };

    struct RiffChunk
    {
        ChunkID chunkID = ChunkID::RIFF;
        uint32_t length;
        std::array<uint8_t, 4> format{};
    };

    struct FormatChunk
    {
        ChunkID chunkID = ChunkID::FMT;
        uint32_t length;
        WavFormat format;
        uint16_t channels;
        uint32_t samplingFreq;
        uint32_t bytesPerSec;
        uint16_t blockSize;
        uint16_t bitsPerSample;
    };

    struct DataChunk
    {
        ChunkID chunkID = ChunkID::DATA;
        uint32_t length;
        std::streamoff offset;
        std::byte pad;
    };
}
