#include "WavParser.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

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
            parseCue();
        }
    }

    void WavParser::parseCue() noexcept
    {
        assert(!m_CueFileName.empty());

        std::ifstream reader(m_CueFileName.data(), std::ios_base::binary);
        std::string line;
        while (std::getline(reader, line)) {
            std::cout << "debug: " << line << std::endl;
        }
    }

    void WavParser::parseWav() noexcept
    {
    }
}
