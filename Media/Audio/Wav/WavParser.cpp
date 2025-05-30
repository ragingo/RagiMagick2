#include "WavParser.h"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <print>
#include <string>
#include <utility>
#include <vector>
#include "Common/BinaryFileReader.h"
#include "Audio/CD/Cue.h"
#include "Audio/CD/CueParser.h"
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

        if (!parseRiffContainer()) {
            return;
        }

        if (!m_CueFileName.empty()) {
            CueParser parser(m_CueFileName);
            m_Cue = parser.parse();
            parseMultiTrackWav();
        }
    }

    bool WavParser::parseRiffContainer() noexcept
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

        if (!m_RiffChunk || !m_FormatChunk || !m_DataChunk) {
            std::println(stderr, "必須チャンク RIFF, fmt, data のいずれかが存在しない");
            return false;
        }

        auto& riff = *m_RiffChunk;
        auto& fmt = *m_FormatChunk;
        auto& data = *m_DataChunk;
        const bool isWave = riff.fileID == FileID::WAVE;
        const auto blockSize = (fmt.bitsPerSample / 8) * std::to_underlying(fmt.channels);

        if (!isWave) {
            std::println("invalid FileID: 0x{0:X}", std::to_underlying(riff.fileID));
        }

        if (!(isWave && (fmt.bitsPerSample == 8 || fmt.bitsPerSample == 16))) {
            std::println(stderr, "WAVE なのに 1 サンプルあたりのビット数が 8 または 16 ではない");
            return false;
        }

        if (fmt.channels != Channel::MONO && fmt.channels != Channel::STEREO) {
            std::println(stderr, "チャンネル数が 1 または 2 ではない");
            return false;
        }

        if (fmt.blockSize != blockSize) {
            std::println(stderr, "ブロック(1サンプル)のサイズがおかしい");
            return false;
        }

        if (fmt.bytesPerSec != fmt.samplingFreq * blockSize) {
            std::println(stderr, "1秒あたりのサイズがおかしい");
            return false;
        }

        return true;
    }

    void WavParser::parseMultiTrackWav() noexcept
    {
        assert(m_Cue);
        assert(m_DataChunk);
        //const auto& cue = *m_Cue;
        const auto& data = *m_DataChunk;

        m_Reader.Seek(data.offset, Common::BinaryFileReader::SeekOrigin::Begin);

        //while (!m_Reader.isEOF()) {
        //}
    }

    void WavParser::parseRiffChunk() noexcept
    {
        RiffChunk riff{};
        m_Reader.ReadUInt32(riff.length);
        m_Reader.ReadUInt32(riff.fileID);
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
        m_DataChunk.emplace(data);
    }
}
