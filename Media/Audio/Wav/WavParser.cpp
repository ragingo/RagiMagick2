#include "WavParser.h"
#include <array>
#include <cassert>
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

        if (m_CueFileName.empty()) {
            parseSingleTrackWav();
        }
        else {
            CueParser parser(m_CueFileName);
            m_Cue = parser.parse();
            parseMultiTrackWav();
        }

    }

    void WavParser::parseSingleTrackWav() noexcept
    {
    }

    void WavParser::parseMultiTrackWav() noexcept
    {
        assert(m_Cue);
        const auto& cue = m_Cue.value();

        auto reader = Common::BinaryFileReader(m_WavFileName, false);
        if (!reader.open()) {
            return;
        }

        WavFileHeader header{};
        reader.ReadBytes(header.fourcc);
        reader.ReadUInt32(header.length);
        reader.ReadBytes(header.format);

        FormatChunk formatChunk{};
        reader.ReadBytes(formatChunk.fourcc);
        reader.ReadUInt32(formatChunk.length);
        reader.ReadUInt16(formatChunk.format);
        reader.ReadUInt16(formatChunk.channels);
        reader.ReadUInt32(formatChunk.samplingFreq);
        reader.ReadUInt32(formatChunk.bytesPerSec);
        reader.ReadUInt16(formatChunk.blockSize);
        reader.ReadUInt16(formatChunk.bitsPerSample);
    }
}
