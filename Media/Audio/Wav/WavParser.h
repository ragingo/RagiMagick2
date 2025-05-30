#pragma once
#include <optional>
#include <string>
#include <string_view>
#include "Common/BinaryFileReader.h"
#include "Audio/CD/Cue.h"
#include "Wav.h"

namespace RagiMagick2::Audio::Wav
{

    class WavParser
    {
    public:
        WavParser(std::string_view wavFileName) noexcept;
        ~WavParser() = default;

        void parse() noexcept;

    private:
        bool parseRiffContainer() noexcept;
        void parseMultiTrackWav() noexcept;
        void parseRiffChunk() noexcept;
        void parseFormatChunk() noexcept;
        void parseDataChunk() noexcept;

        std::string m_WavFileName;
        std::string m_CueFileName;
        std::optional<Cue> m_Cue;
        std::optional<RiffChunk> m_RiffChunk;
        std::optional<FormatChunk> m_FormatChunk;
        std::optional<DataChunk> m_DataChunk;
        Common::BinaryFileReader m_Reader;
    };

} // namespace RagiMagick2::Audio::Wav
