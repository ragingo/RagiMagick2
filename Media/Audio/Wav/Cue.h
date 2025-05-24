#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace RagiMagick2::Audio::Wav
{
    enum class CueCommand
    {
        REM,
        PERFORMAR,
        TITLE,
        FILE,
        TRACK,
        INDEX
    };

    enum class CueRemarkType
    {
        GENRE,
        DATE,
        DISCID,
        COMMENT,
        COMPOSER,
        UNKNOWN
    };

    struct CueRemark
    {
        CueRemarkType type;
        std::string value;
    };

    struct Cue
    {
        std::string performer;
        std::string title;
        std::string composer;
        std::string wavFileName;
        std::vector<CueRemark> remarks{};
    };

    struct CueTrackIndex
    {
        uint32_t index;
        std::string time;
    };

    struct CueTrack
    {
        uint32_t id;
        std::string title;
        std::string performer;
        std::string composer;
        std::vector<CueTrackIndex> indices{};
    };

}
