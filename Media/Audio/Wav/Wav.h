#pragma once
#include <cstddef>
#include <cstdint>
#include <ios>
#include <string>
#include <type_traits>

namespace RagiMagick2::Audio::Wav
{
    enum class FileID : uint32_t
    {
        UNKNOWN = 0,
        WAVE = 0x45564157, // 'WAVE' -> 'EVAW'
    };

    enum class WaveFormat : uint16_t
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

    enum class Channel: uint16_t
    {
        MONO = 1,
        STEREO = 2
    };

    template<typename T>
        requires std::is_same_v<T, uint8_t> || std::is_same_v<T, int16_t>
    struct MonoData
    {
        T value;
    };

    template<typename T>
        requires std::is_same_v<T, uint8_t> || std::is_same_v<T, int16_t>
    struct StereoData
    {
        T left;
        T right;
    };

    struct RiffChunk
    {
        ChunkID chunkID = ChunkID::RIFF;
        uint32_t length;
        FileID fileID;
    };

    struct FormatChunk
    {
        ChunkID chunkID = ChunkID::FMT;
        uint32_t length;
        WaveFormat format;
        Channel channels;
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

    struct Track
    {
        uint32_t id;
        std::string title;
        uint32_t preGapOffset;
        uint32_t soundOffset;
        uint32_t soundLength;
    };
}
