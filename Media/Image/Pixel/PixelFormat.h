#pragma once

namespace RagiMagick2::Image::Pixel
{
    enum class PixelFormat
    {
        // YCbCr 4:4:4
        YCBCR444_UINT,
        // YCbCr 4:2:0
        YCBCR420_UINT,
        // BGRA 32bit
        B8G8R8A8_UINT,
    };
} // namespace RagiMagick2::Image::Pixel
