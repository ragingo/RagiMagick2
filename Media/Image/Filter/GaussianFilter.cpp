#include "GaussianFilter.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>
#include "IImageFilter.h"

namespace
{
    constexpr std::array<float, 3 * 3> SMALL_KERNEL = {
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
        2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f
    };

    constexpr std::array<float, 5 * 5> LARGE_KERNEL = {
        1.0f / 256.0f,  4.0f / 256.0f,  6.0f / 256.0f,  4.0f / 256.0f, 1.0f / 256.0f,
        4.0f / 256.0f, 16.0f / 256.0f, 24.0f / 256.0f, 16.0f / 256.0f, 4.0f / 256.0f,
        6.0f / 256.0f, 24.0f / 256.0f, 36.0f / 256.0f, 24.0f / 256.0f, 6.0f / 256.0f,
        4.0f / 256.0f, 16.0f / 256.0f, 24.0f / 256.0f, 16.0f / 256.0f, 4.0f / 256.0f,
        1.0f / 256.0f,  4.0f / 256.0f,  6.0f / 256.0f,  4.0f / 256.0f, 1.0f / 256.0f,
    };

    template <size_t KERNEL_SIZE, size_t N = KERNEL_SIZE * KERNEL_SIZE>
    constexpr std::array<float, N> createKernel()
    {
        static_assert(KERNEL_SIZE == 3 || KERNEL_SIZE == 5);
        if constexpr (KERNEL_SIZE == 3) {
            return SMALL_KERNEL;
        }
        else if constexpr (KERNEL_SIZE == 5) {
            return LARGE_KERNEL;
        }
        else {
            return {};
        }
    }

    constexpr int KERNEL_SIZE = 5; // 3 or 5
    constexpr int KERNEL_CENTER = KERNEL_SIZE / 2;
    constexpr auto KERNEL = createKernel<KERNEL_SIZE>();
}

namespace RagiMagick2::Image::Filter
{
    ImageInfo GaussianFilter::apply(const ImageInfo& src) noexcept
    {
        assert(src.componentCount >= 3);

        ImageInfo dst{};
        dst.width = src.width;
        dst.height = src.height;
        dst.componentCount = src.componentCount;
        dst.pixels.resize(src.pixels.size());

        for (int row = 0; row < src.height; ++row) {
            for (int col = 0; col < src.width; ++col) {
                float r = 0;
                float g = 0;
                float b = 0;

                for (int k = 0; k < KERNEL_SIZE * KERNEL_SIZE; ++k) {
                    const int sx = col + k % KERNEL_SIZE - KERNEL_CENTER;
                    const int sy = row + k / KERNEL_SIZE - KERNEL_CENTER;
                    if (sx < 0 || sx >= src.width || sy < 0 || sy >= src.height) {
                        continue;
                    }
                    const size_t index = static_cast<size_t>(sy * src.width + sx) * src.componentCount;
                    r += src.pixels[index + 0] * KERNEL[k];
                    g += src.pixels[index + 1] * KERNEL[k];
                    b += src.pixels[index + 2] * KERNEL[k];
                }

                const size_t index = static_cast<size_t>(row * src.width + col) * src.componentCount;
                dst.pixels[index + 0] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
                dst.pixels[index + 1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
                dst.pixels[index + 2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
                dst.pixels[index + 3] = src.pixels[index + 3];
            }
        }

        return dst;
    }
} // namespace RagiMagick2::Image::Filter
