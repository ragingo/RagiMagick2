#include "WavParser.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include "Common/BinaryFileReader.h"
#include "Cue.h"
#include "CueParser.h"
#include "Wav.h"

namespace RagiMagick2::Audio::Wav
{
    WavParser::WavParser(std::string_view wavFileName) noexcept
        : m_WavFileName(std::string(wavFileName))
        , m_Reader(Common::BinaryFileReader(m_WavFileName, false))
    {
    }

    void WavParser::parse() noexcept
    {
        // .cue 存在チェック
        auto wavFilePath = std::filesystem::path(m_WavFileName);
        auto cueFilePath = wavFilePath.replace_extension("cue");
        if (std::filesystem::exists(cueFilePath)) {
            m_CueFileName = cueFilePath.string();
        }


        if (!m_Reader.open()) {
            return;
        }

        parseRiffContainer();

        if (!m_CueFileName.empty()) {
            CueParser parser(m_CueFileName);
            m_Cue = parser.parse();
            parseMultiTrackWav();
        }
    }

    void WavParser::parseRiffContainer() noexcept
    {
        while (!m_Reader.isEOF()) {
            ChunkID chunkID;
            m_Reader.ReadUInt32(chunkID);

            switch (chunkID) {
            case ChunkID::RIFF:
                parseRiffChunk();
                break;
            case ChunkID::FMT:
                parseFormatChunk();
                break;
            case ChunkID::DATA:
                parseDataChunk();
                break;
            }
        }
    }

    void WavParser::parseMultiTrackWav() noexcept
    {
        assert(m_Cue);
        //const auto& cue = m_Cue.value();
    }

    void WavParser::parseRiffChunk() noexcept
    {
        RiffChunk riff{};
        m_Reader.ReadUInt32(riff.length);
        m_Reader.ReadBytes(riff.format);
        m_RiffChunk.emplace(riff);
    }

    void WavParser::parseFormatChunk() noexcept
    {
        FormatChunk fmt{};
        m_Reader.ReadUInt32(fmt.length);
        m_Reader.ReadUInt16(fmt.format);
        m_Reader.ReadUInt16(fmt.channels);
        m_Reader.ReadUInt32(fmt.samplingFreq);
        m_Reader.ReadUInt32(fmt.bytesPerSec);
        m_Reader.ReadUInt16(fmt.blockSize);
        m_Reader.ReadUInt16(fmt.bitsPerSample);
        m_FormatChunk.emplace(fmt);
    }

    void WavParser::parseDataChunk() noexcept
    {
        DataChunk data{};
        m_Reader.ReadUInt32(data.length);
        data.offset = m_Reader.GetCurrentPosition();
        m_Reader.Seek(data.length, Common::BinaryFileReader::SeekOrigin::Current);
        m_Reader.ReadUInt8(data.pad);
    }
}
