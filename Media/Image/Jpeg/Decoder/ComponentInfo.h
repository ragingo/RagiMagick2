#pragma once
#include <cstdint>
#include <vector>
#include "Image/Jpeg/Syntax/Segment.h"

namespace RagiMagick2::Image::Jpeg
{
    struct ComponentInfo
    {
        Syntax::ComponentID id;
        int horizontalSamplingFactor;
        int verticalSamplingFactor;
        int width;
        int height;
        std::vector<int16_t> buffer;
    };
} // namespace RagiMagick2::Image::Jpeg
