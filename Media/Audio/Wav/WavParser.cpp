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

        auto reader = Common::BinaryFileReader(m_WavFileName);
        if (!reader.open()) {
            return;
        }

        {
            WavFileHeader header{};
            reader.ReadBytes(header.fourcc);
            //std::array<decltype(header.length), 1> lengthBuf{};
            //reader.ReadUInt32(header.length); // ReadUInt32 を作る
        }
    }
}
