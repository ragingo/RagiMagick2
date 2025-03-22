#pragma once
#include <cassert>
#include <cstdint>
#include <string_view>
#include <vector>
#include <fstream>

namespace RagiMagick2::Image::Bitmap
{

#pragma pack(push, 1)

    struct BitmapFileHeader
    {
        uint16_t bfType;
        uint32_t bfSize;
        uint16_t bfReserved1;
        uint16_t bfReserved2;
        uint32_t bfOffBits;
    };

    struct BitmapInfoHeader
    {
        uint32_t biSize;
        int32_t biWidth;
        int32_t biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        int32_t biXPelsPerMeter;
        int32_t biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t biClrImportant;
    };

#pragma pack(pop)

    void writeBitmap(
        const std::string_view filename,
        int width,
        int height,
        int bitCount,
        const std::vector<uint8_t>& pixels
    )
    {
        assert(bitCount == 32);

        BitmapFileHeader fileHeader{};
        fileHeader.bfType = 0x4D42;
        fileHeader.bfSize = static_cast<uint32_t>(sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + pixels.size());
        fileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);

        BitmapInfoHeader infoHeader{};
        infoHeader.biSize = sizeof(BitmapInfoHeader);
        infoHeader.biWidth = width;
        infoHeader.biHeight = -height;
        infoHeader.biPlanes = 1;
        infoHeader.biBitCount = bitCount;
        infoHeader.biSizeImage = static_cast<uint32_t>(pixels.size());

        std::ofstream file(filename.data(), std::ios::binary);
        if (!file) {
            return;
        }

        file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
        file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));
        file.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
    }

} // namespace RagiMagick2::Image::Bitmap
