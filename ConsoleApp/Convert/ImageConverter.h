#pragma once

#include <print>
#include <string_view>
#include <type_traits>
#include <vector>
#include "CommandLine/Options.h"
#include "Image/Bitmap/Bitmap.h"
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

        auto decoder = JpegDecoder(m_InputFile);
        DecodeResult result{};
        decoder.decode(result);
        writeBitmap(m_OutputFile, result.width, result.height, 32, result.pixels);
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
        return Unknown;
    }

private:
    std::vector<std::string_view> m_Options;
    std::string_view m_InputFile;
    std::string_view m_OutputFile;
    std::string_view m_OutputFormat;
};
