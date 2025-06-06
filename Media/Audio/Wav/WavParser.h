#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <vector>
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

        bool parse() noexcept;

        bool hasCue() const noexcept { return !!m_Cue; }

        const std::string& getWavFileName() const noexcept { return m_WavFileName; }
        const std::string& getCueFileName() const noexcept { return m_CueFileName; }
        const std::optional<CD::Cue>& getCue() const noexcept { return m_Cue; }
        const std::vector<Track>& getTracks() const noexcept { return m_Tracks; }

    private:
        bool parseRiffContainer() noexcept;
        void parseMultiTrackWav() noexcept;
        void parseRiffChunk() noexcept;
        void parseFormatChunk() noexcept;
        void parseDataChunk() noexcept;

        std::string m_WavFileName;
        std::string m_CueFileName;
        std::optional<CD::Cue> m_Cue;
        std::optional<RiffChunk> m_RiffChunk;
        std::optional<FormatChunk> m_FormatChunk;
        std::optional<DataChunk> m_DataChunk;
        Common::BinaryFileReader m_Reader;
        std::vector<Track> m_Tracks{};
    };

} // namespace RagiMagick2::Audio::Wav
