#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace RagiMagick2::Audio::Wav
{
    struct Cue
    {
        std::string genre;
        std::string date;
        std::string discID;
        std::string comment;
        std::string performer;
        std::string title;
        std::string composer;
        std::string wavFileName;
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
        std::vector<CueTrackIndex> indices;
    };
}
