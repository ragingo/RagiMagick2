#pragma once
#include <optional>
#include <string>
#include <string_view>
#include "Cue.h"

namespace RagiMagick2::Audio::Wav
{

    class WavParser
    {
    public:
        WavParser(std::string_view wavFileName) noexcept;
        ~WavParser() = default;

        void parse() noexcept;

    private:
        void parseSingleTrackWav() noexcept;
        void parseMultiTrackWav() noexcept;

        std::string m_WavFileName;
        std::string m_CueFileName;
        std::optional<Cue> m_Cue;
    };

} // namespace RagiMagick2::Audio::Wav
