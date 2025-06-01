#pragma once

enum class Command
{
    Convert,
    Show,
    Help,
    Unknown
};

enum class ImageConverterOption
{
    InputFile,
    OutputFile,
    OutputFormat,
    Filter,
    Help,
    Unknown
};

enum class AudioShowOption
{
    InputFile,
    Help,
    Unknown
};
