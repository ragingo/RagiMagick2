#pragma once
#include "IImageFilter.h"

namespace RagiMagick2::Image::Filter
{
    class GrayscaleFilter : public IImageFilter
    {
    public:
        ImageInfo apply(const ImageInfo& src) noexcept override;
    };
} // namespace RagiMagick2::Image::Filter

