#pragma once
#include <optional>
#include <string>
#include <string_view>

namespace RagiMagick2::Audio::Wav
{

    class WavParser
    {
    public:
        WavParser(std::string_view wavFileName) noexcept;
        ~WavParser() = default;

        void parse() noexcept;

    private:
        void parseCue() noexcept;
        void parseWav() noexcept;

        std::string m_WavFileName;
        std::string m_CueFileName;
    };

} // namespace RagiMagick2::Audio::Wav
