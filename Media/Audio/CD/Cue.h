#pragma once
#include <cstdint>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

namespace RagiMagick2::Audio::CD
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
        int32_t minutes;
        int32_t seconds;
        int32_t frames;
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

    inline std::tuple<int32_t, int32_t, int32_t> getTrackIndexTime(std::string value) noexcept
    {
        int32_t minutes = 0;
        int32_t seconds = 0;
        int32_t frames = 0;
        const auto& fields = value
            | std::ranges::views::split(':')
            | std::ranges::views::transform([](const auto& field) {
                return std::stoi(std::string(field.begin(), field.end()));
              })
            | std::ranges::to<std::vector>();

        if (fields.size() == 3) {
            minutes = fields[0];
            seconds = fields[1];
            frames = fields[2];
        }

        return std::make_tuple(minutes, seconds, frames);
    }

    inline bool isPreGap(const CueTrackIndex& index) noexcept
    {
        return index.index == 0;
    }

}
