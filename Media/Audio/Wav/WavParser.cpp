#include "WavParser.h"
#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "Cue.h"
#include "CueParser.h"

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

        if (!m_CueFileName.empty()) {
            CueParser parser(m_CueFileName);
            parser.parse();
        }
    }

    void WavParser::parseWav() noexcept
    {
    }
}
