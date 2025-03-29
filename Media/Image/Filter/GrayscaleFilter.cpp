#include "GrayscaleFilter.h"
#include <cassert>
#include <cstdint>
#include <vector>
#include "IImageFilter.h"

namespace RagiMagick2::Image::Filter
{
    ImageInfo GrayscaleFilter::apply(const ImageInfo& src) noexcept
    {
        assert(src.componentCount >= 3);

        ImageInfo dst{};
        dst.width = src.width;
        dst.height = src.height;
        dst.componentCount = src.componentCount;
        dst.pixels.resize(src.pixels.size());

        for (size_t i = 0; i < src.pixels.size(); i += src.componentCount) {
            uint8_t r = src.pixels[i + 0];
            uint8_t g = src.pixels[i + 1];
            uint8_t b = src.pixels[i + 2];
            uint8_t gray = (r + g + b) / 3;
            dst.pixels[i + 0] = gray;
            dst.pixels[i + 1] = gray;
            dst.pixels[i + 2] = gray;
        }

        return dst;
    }
} // namespace RagiMagick2::Image::Filter
