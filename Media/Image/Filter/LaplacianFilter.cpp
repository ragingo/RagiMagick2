#include "LaplacianFilter.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>
#include "IImageFilter.h"

namespace
{
    template <size_t KERNEL_SIZE, size_t N = KERNEL_SIZE * KERNEL_SIZE>
    constexpr std::array<int, N> createKernel()
    {
        std::array<int, N> kernel{};
        for (int i = 0; i < N; ++i) {
            kernel[i] = -1;
        }
        if (KERNEL_SIZE == 3) {
            kernel[N / 2] = 8;
        }
        else if (KERNEL_SIZE == 5) {
            kernel[N / 2] = 24;
        }
        return kernel;
    }

    constexpr int KERNEL_SIZE = 5; // 3 or 5
    constexpr int KERNEL_CENTER = KERNEL_SIZE / 2;
    constexpr auto KERNEL = createKernel<KERNEL_SIZE>();
}

namespace RagiMagick2::Image::Filter
{
    ImageInfo LaplacianFilter::apply(const ImageInfo& src) noexcept
    {
        assert(src.componentCount >= 3);

        ImageInfo dst{};
        dst.width = src.width;
        dst.height = src.height;
        dst.componentCount = src.componentCount;
        dst.pixels.resize(src.pixels.size());

        for (size_t row = 0; row < src.height; ++row) {
            for (size_t col = 0; col < src.width; ++col) {
                int r = 0;
                int g = 0;
                int b = 0;

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

                const size_t index = (row * src.width + col) * src.componentCount;
                dst.pixels[index + 0] = static_cast<uint8_t>(std::clamp(r, 0, 255));
                dst.pixels[index + 1] = static_cast<uint8_t>(std::clamp(g, 0, 255));
                dst.pixels[index + 2] = static_cast<uint8_t>(std::clamp(b, 0, 255));
            }
        }

        return dst;
    }
} // namespace RagiMagick2::Image::Filter
