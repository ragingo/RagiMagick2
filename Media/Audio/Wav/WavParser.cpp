#include "WavParser.h"
#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>
#include "Cue.h"


// Space Separated Value Line Parser
// スペース区切りの行に特化したパーサー
class SSVLineParser final
{
public:
    SSVLineParser(const std::string& separator = " ") noexcept
        : m_Separator(separator)
    {
    }

    std::vector<std::string> parse(std::string_view text) noexcept
    {
        const std::string_view::const_pointer head = text.data();
        const std::string_view::const_pointer tail = head + text.size();
        std::string_view::const_pointer ptr = head;
        std::string_view::const_pointer startPtr = head;
        std::string_view::const_pointer endPtr = nullptr;

        std::vector<std::string> tokens;
        bool beginTrim = true;
        bool isQuotedValue = false;

        while (ptr < tail) {
            if (*ptr == ' ' && beginTrim) {
                ++ptr;
                startPtr = ptr;
                continue;
            }
            else {
                beginTrim = false;
            }

            if (*ptr == '\r') {
                if (auto next = ptr + 1; next < tail && *next == '\n') {
                    ++ptr;
                    continue;
                }
            }

            switch (*ptr) {
            case '\r':
            case '\n':
                if (ptr != startPtr) {
                    endPtr = ptr - 1;
                }
                break;
            case '\"':
                isQuotedValue = !isQuotedValue;
                if (isQuotedValue) {
                    startPtr = ptr;
                }
                else {
                    endPtr = ptr;
                }
                break;
            case ' ':
                if (!isQuotedValue) {
                    endPtr = ptr - 1;
                }
                break;
            default:
                break;
            }

            if (startPtr && endPtr) {
                tokens.emplace_back(text.substr(startPtr - head, (endPtr - startPtr) + 1));
                startPtr = ptr + 1;
                endPtr = nullptr;
                beginTrim = true;
            }

            ++ptr;
        }

        return tokens;
    }

private:
    std::string m_Separator;
};


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

        std::array<uint8_t, 3> bom;
        reader.read(reinterpret_cast<char*>(bom.data()), bom.size());
        if (bom != std::array<uint8_t, 3>{ 0xef, 0xbb, 0xbf }) {
            reader.seekg(0, std::ios_base::beg);
        }

        Cue cue{};
        std::string line;
        SSVLineParser lineParser;
        while (std::getline(reader, line)) {
            auto values = lineParser.parse(line);

            std::cout << "===== debug: " << line << std::endl;
            for (auto&& value : values) {
                std::cout << value << std::endl;
            }
        }
    }

    void WavParser::parseWav() noexcept
    {
    }
}
