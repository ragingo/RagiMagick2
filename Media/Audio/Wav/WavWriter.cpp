#include "WavWriter.h"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <ios>
#include <print>
#include <string>
#include <utility>
#include "Wav.h"

namespace RagiMagick2::Audio::Wav
{
    WavWriter::WavWriter(const std::string& fileName) noexcept
    {
        m_FileName = fileName;
    }

    bool WavWriter::open() noexcept
    {
        m_Writer = std::ofstream{ m_FileName, std::ios_base::binary };

        if (!m_Writer.is_open()) {
            std::println(stderr, "ファイル {} のオープンに失敗", m_FileName);
            return false;
        }

        return true;
    }

    void WavWriter::writeRiffChunk(uint32_t trackSize) noexcept
    {
        uint32_t riffLength = sizeof(FileID) + sizeof(FormatChunk) + sizeof(DataChunkHeader) + trackSize;
        constexpr uint32_t fileID = std::to_underlying(FileID::WAVE);

        m_Writer.write("RIFF", 4);
        m_Writer.write(reinterpret_cast<const char*>(&riffLength), sizeof(riffLength));
        m_Writer.write(reinterpret_cast<const char*>(&fileID), sizeof(fileID));
    }

    void WavWriter::writeFormatChunk(const FormatChunk& formatChunk) noexcept
    {
        m_Writer.write(reinterpret_cast<const char*>(&formatChunk), sizeof(formatChunk));
    }

    void WavWriter::writeDataChunkHeader(uint32_t trackSize) noexcept
    {
        m_Writer.write("data", 4);
        m_Writer.write(reinterpret_cast<const char*>(&trackSize), sizeof(trackSize));
    }

    void WavWriter::writePCM(uint8_t* data, size_t count) noexcept
    {
        m_Writer.write(reinterpret_cast<const char*>(data), count);
    }

    void WavWriter::close() noexcept
    {
        m_Writer.flush();
        m_Writer.close();
    }
}
