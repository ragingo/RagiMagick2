#pragma once
#include <cstdint>
#include <fstream>
#include <string>
#include "Wav.h"

namespace RagiMagick2::Audio::Wav
{

    class WavWriter final
    {
    public:
        WavWriter(const std::string& fileName) noexcept;
        bool open() noexcept;
        void writeRiffChunk(uint32_t trackSize) noexcept;
        void writeFormatChunk(const FormatChunk& riffChunk) noexcept;
        void writeDataChunkHeader(uint32_t trackSize) noexcept;
        void writePCM(uint8_t* data, size_t count) noexcept;
        void close() noexcept;

    private:
        std::string m_FileName;
        std::ofstream m_Writer;
    };

} // namespace RagiMagick2::Audio::Wav
