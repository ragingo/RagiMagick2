#pragma once
#include <memory>
#include <print>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <vector>
#include "CommandLine/Options.h"
#include "Image/Bitmap/Bitmap.h"
#include "Image/Filter/IImageFilter.h"
#include "Image/Filter/BinaryFilter.h"
#include "Image/Filter/GaussianFilter.h"
#include "Image/Filter/GrayscaleFilter.h"
#include "Image/Filter/LaplacianFilter.h"
#include "Image/Filter/MosaicFilter.h"
#include "Image/Jpeg/Decoder/JpegDecoder.h"

class ImageConverter final
{
public:
    ImageConverter(std::vector<std::string_view> options) noexcept
    {
        m_Options = std::move(options);
    }

    bool parse() noexcept
    {
        for (size_t i = 0; i < m_Options.size(); ++i) {
            auto& option = m_Options[i];
            switch (toOption(option)) {
            case ImageConverterOption::InputFile:
                m_InputFile = (i + 1 < m_Options.size()) ? m_Options[++i] : "";
                break;
            case ImageConverterOption::OutputFile:
                m_OutputFile = (i + 1 < m_Options.size()) ? m_Options[++i] : "";
                break;
            case ImageConverterOption::OutputFormat:
                m_OutputFormat = (i + 1 < m_Options.size()) ? m_Options[++i] : "";
                break;
            case ImageConverterOption::Filter:
                m_Filters = toFilters((i + 1 < m_Options.size()) ? m_Options[++i] : "");
                break;
            default:
                break;
            }
        }

        return !m_InputFile.empty() && !m_OutputFile.empty();
    }

    bool execute() noexcept
    {
        using namespace RagiMagick2::Image::Bitmap;
        using namespace RagiMagick2::Image::Jpeg;
        using namespace RagiMagick2::Image::Filter;

        ImageInfo imageInfo{};
        if (m_InputFile.ends_with(".jpg") || m_InputFile.ends_with(".jpeg")) {
            imageInfo = decodeJpeg(m_InputFile);
        }
        else {
            return false;
        }
        
        for (auto& filter : m_Filters) {
            imageInfo = filter->apply(imageInfo);
        }

        writeBitmap(m_OutputFile, imageInfo.width, imageInfo.height, imageInfo.componentCount * 8, imageInfo.pixels);
        return true;
    }

private:
    ImageConverterOption toOption(std::string_view option) const noexcept
    {
        using enum ImageConverterOption;
        if (option == "--input-file" || option == "-i") {
            return InputFile;
        }
        if (option == "--output-file" || option == "-o") {
            return OutputFile;
        }
        if (option == "--output-format" || option == "-of") {
            return OutputFormat;
        }
        if (option == "--filter") {
            return Filter;
        }
        return Unknown;
    }

    std::vector<std::shared_ptr<RagiMagick2::Image::Filter::IImageFilter>> toFilters(std::string_view option) const noexcept
    {
        using namespace RagiMagick2::Image::Filter;
        std::vector<std::shared_ptr<IImageFilter>> filters;

        for (const auto& value : std::views::split(option, ',')) {
            const auto filter = std::string_view{ value.begin(), value.end() };
            if (filter == "binary") {
                filters.emplace_back(std::make_shared<BinaryFilter>());
            }
            else if (filter == "gaussian") {
                filters.emplace_back(std::make_shared<GaussianFilter>());
            }
            else if (filter == "grayscale") {
                filters.emplace_back(std::make_shared<GrayscaleFilter>());
            }
            else if (filter == "laplacian") {
                filters.emplace_back(std::make_shared<LaplacianFilter>());
            }
            else if (filter == "mosaic") {
                filters.emplace_back(std::make_shared<MosaicFilter>());
            }
        }
        return filters;
    }

    RagiMagick2::Image::Filter::ImageInfo decodeJpeg(std::string_view fileName) const noexcept
    {
        using namespace RagiMagick2::Image::Jpeg;
        auto decoder = JpegDecoder(fileName);
        DecodeResult result{};
        decoder.decode(result);
        return { result.width, result.height, 4, result.pixels };
    }

private:
    std::vector<std::string_view> m_Options;
    std::string_view m_InputFile;
    std::string_view m_OutputFile;
    std::string_view m_OutputFormat;
    std::vector<std::shared_ptr<RagiMagick2::Image::Filter::IImageFilter>> m_Filters;
};
