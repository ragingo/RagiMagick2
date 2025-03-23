#include "BinaryFilter.h"
#include <cstdint>
#include <vector>
#include "IImageFilter.h"

namespace
{
    constexpr uint8_t THRESHOLD = 128;
}

namespace RagiMagick2::Image::Filter
{
    ImageInfo BinaryFilter::apply(const ImageInfo& src) noexcept
    {
        ImageInfo dst{};
        dst.width = src.width;
        dst.height = src.height;
        dst.componentCount = src.componentCount;
        dst.pixels.resize(src.pixels.size());

        for (size_t i = 0; i < src.pixels.size(); i += src.componentCount) {
            uint8_t r = src.pixels[i + 0];
            uint8_t g = src.pixels[i + 1];
            uint8_t b = src.pixels[i + 2];
            uint8_t a = src.pixels[i + 3];
            uint8_t gray = (r + g + b) / 3;
            uint8_t binary = gray > THRESHOLD ? 255 : 0;
            dst.pixels[i + 0] = binary;
            dst.pixels[i + 1] = binary;
            dst.pixels[i + 2] = binary;
            dst.pixels[i + 3] = a;
        }

        return dst;
    }
} // namespace RagiMagick2::Image::Filter
