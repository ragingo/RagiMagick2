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
        PERFORMER,
        TITLE,
        FILE,
        TRACK,
        INDEX,
        UNKNOWN
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

    struct CueTrackIndex
    {
        uint32_t index;
        std::string time;
    };

    struct CueTrack
    {
        uint32_t id;
        std::string type;
        std::string title;
        std::string performer;
        std::vector<CueTrackIndex> indices{};
        std::vector<CueRemark> remarks{};
    };

    struct Cue
    {
        std::string performer;
        std::string title;
        std::string wavFileName;
        std::vector<CueTrack> tracks{};
        std::vector<CueRemark> remarks{};
    };

}
