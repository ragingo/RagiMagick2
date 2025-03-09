#pragma once
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>
#include "Image/Jpeg/Syntax/Marker.h"
#include "Image/Jpeg/Syntax/Segment.h"
#include "Common/BinaryFileReader.h"

namespace RagiMagick2::Image::Jpeg
{
    class JpegParser final
    {
    public:
        JpegParser(std::string_view filename) noexcept;
        ~JpegParser();

        bool parse() noexcept;

        inline auto getMarkers() const noexcept { return m_Markers; }
        inline auto getSegments() const noexcept { return m_Segments; }
        inline auto getECS() const noexcept { return m_ECS; }

    private:
        void parseSOI();
        void parseAPP0();
        void parseDQT();
        void parseSOF0();
        void parseEOI();
        void parseDHT();
        void parseSOS();
        void parseECS();
        void parseDRI();
        void parseAPP1();
        void parseAPP2();
        void parseAPP13();
        void parseAPP14();
        void parseCOM();

    private:
        Common::BinaryFileReader m_FileReader;
        std::vector<Syntax::Marker> m_Markers{};
        std::vector<std::shared_ptr<Syntax::Segment>> m_Segments{};
        std::vector<uint8_t> m_ECS{};
    };
} // namespace RagiMagick2::Image::Jpeg
