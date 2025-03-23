#pragma once
#include <algorithm>
#include <memory>
#include <print>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <vector>
#include "CommandLine/Options.h"
#include "Image/Bitmap/Bitmap.h"
#include "Image/Jpeg/Decoder/JpegDecoder.h"
#include "Image/Filter/IImageFilter.h"
#include "Image/Filter/BinaryFilter.h"

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

        auto decoder = JpegDecoder(m_InputFile);
        DecodeResult result{};
        decoder.decode(result);

        ImageInfo imageInfo = { result.width, result.height, 4, result.pixels };
        
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

    RagiMagick2::Image::Filter::FilterType toFilterType(std::string_view filter) const noexcept
    {
        using namespace RagiMagick2::Image::Filter;
        if (filter == "binary") {
            return FilterType::Binary;
        }
        return FilterType::Unknown;
    }

    std::vector<std::shared_ptr<RagiMagick2::Image::Filter::IImageFilter>> toFilters(std::string_view option) const noexcept
    {
        using namespace RagiMagick2::Image::Filter;
        std::vector<std::shared_ptr<IImageFilter>> filters;

        for (const auto& filter : std::views::split(option, ',')) {
            switch (toFilterType(std::string_view{ filter.begin(), filter.end() })) {
            case FilterType::Binary:
                filters.emplace_back(std::make_shared<BinaryFilter>());
                break;
            default:
                break;
            }
        }
        return filters;
    }

private:
    std::vector<std::string_view> m_Options;
    std::string_view m_InputFile;
    std::string_view m_OutputFile;
    std::string_view m_OutputFormat;
    std::vector<std::shared_ptr<RagiMagick2::Image::Filter::IImageFilter>> m_Filters;
};
