#pragma once
#include <cstdint>
#include <vector>

namespace RagiMagick2::Image::Filter
{
    struct ImageInfo
    {
        int width;
        int height;
        int componentCount;
        std::vector<uint8_t> pixels;
    };

    class IImageFilter
    {
    public:
        virtual ~IImageFilter() = default;
        virtual ImageInfo apply(const ImageInfo& src) noexcept = 0;
    };

    enum class FilterType
    {
        Binary,
        Gaussian,
        Grayscale,
        Laplacian,
        Mosaic,
        Unknown
    };
} // namespace RagiMagick2::Image::Filter
