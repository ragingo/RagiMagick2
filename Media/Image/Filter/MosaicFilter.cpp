#include "MosaicFilter.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>
#include "IImageFilter.h"

namespace
{
    constexpr int BLOCK_SIZE = 30;
    constexpr int PIXEL_COUNT_PER_BLOCK = BLOCK_SIZE * BLOCK_SIZE;
}

namespace RagiMagick2::Image::Filter
{
    ImageInfo MosaicFilter::apply(const ImageInfo& src) noexcept
    {
        assert(src.componentCount >= 3);

        ImageInfo dst{};
        dst.width = src.width;
        dst.height = src.height;
        dst.componentCount = src.componentCount;
        dst.pixels.resize(src.pixels.size());

        const auto blockCountX = src.width / BLOCK_SIZE;
        const auto blockCountY = src.height / BLOCK_SIZE;

        for (size_t blockY = 0; blockY < blockCountY; ++blockY) {
            for (size_t blockX = 0; blockX < blockCountX; ++blockX) {
                uint32_t r = 0;
                uint32_t g = 0;
                uint32_t b = 0;
                for (size_t row = 0; row < BLOCK_SIZE; ++row) {
                    for (size_t col = 0; col < BLOCK_SIZE; ++col) {
                        const size_t x = blockX * BLOCK_SIZE + col;
                        const size_t y = blockY * BLOCK_SIZE + row;
                        const size_t index = (y * src.width + x) * src.componentCount;
                        r += src.pixels[index + 0];
                        g += src.pixels[index + 1];
                        b += src.pixels[index + 2];
                    }
                }

                r /= PIXEL_COUNT_PER_BLOCK;
                g /= PIXEL_COUNT_PER_BLOCK;
                b /= PIXEL_COUNT_PER_BLOCK;

                for (size_t row = 0; row < BLOCK_SIZE; ++row) {
                    for (size_t col = 0; col < BLOCK_SIZE; ++col) {
                        const size_t x = blockX * BLOCK_SIZE + col;
                        const size_t y = blockY * BLOCK_SIZE + row;
                        const size_t index = (y * dst.width + x) * dst.componentCount;
                        dst.pixels[index + 0] = static_cast<uint8_t>(std::clamp(r, 0u, 255u));
                        dst.pixels[index + 1] = static_cast<uint8_t>(std::clamp(g, 0u, 255u));
                        dst.pixels[index + 2] = static_cast<uint8_t>(std::clamp(b, 0u, 255u));
                    }
                }
            }
        }

        return dst;
    }
} // namespace RagiMagick2::Image::Filter
