#pragma once
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdint>
#include <format>
#include <print>
#include <string>
#include <string_view>
#include <vector>
#include "Common/BinaryFileReader.h"
#include "Wav.h"
#include "WavParser.h"
#include "WavWriter.h"

namespace RagiMagick2::Audio::Wav
{
    class WavSplitter final
    {
    public:
        WavSplitter(std::string_view inputFile, std::string_view outputDir)
            : m_InputFile(inputFile)
            , m_OutputDir(outputDir)
        {
        }

        bool execute()
        {
            WavParser parser(m_InputFile);
            if (!parser.parse()) {
                std::println(stderr, "WAVファイルのパースに失敗しました");
                return false;
            }
            if (!parser.hasCue()) {
                std::println(stderr, "CUEシートが見つかりません");
                return false;
            }
            const auto& tracks = parser.getTracks();
            const auto& formatChunk = parser.getFormatChunk();
            const auto& dataChunk = parser.getDataChunk();
            if (!formatChunk || !dataChunk) {
                std::println(stderr, "WAVチャンク情報が不正です");
                return false;
            }

            auto reader = Common::BinaryFileReader(m_InputFile, false);
            if (!reader.open()) {
                std::println(stderr, "{} のオープンに失敗した", m_InputFile);
                return false;
            }

            for (const auto& track : tracks) {
                if (reader.isEOF()) {
                    break;
                }
                if (track.soundOffset >= dataChunk.value().header.length) {
                    std::println(stderr, "トラック {} のオフセットがデータ長を超えている", track.id);
                    continue;
                }
                reader.Seek(track.soundOffset, Common::BinaryFileReader::SeekOrigin::Begin);
                std::string fileName = std::format("{}/{:02}_{}.wav", m_OutputDir, track.id, track.title);

                WavWriter writer(fileName);
                if (!writer.open()) {
                    std::println(stderr, "{} のオープンに失敗した", fileName);
                    continue;
                }
                writer.writeRiffChunk(track.soundLength);
                writer.writeFormatChunk(formatChunk.value());
                writer.writeDataChunkHeader(track.soundLength);

                std::array<uint8_t, 1024> buffer{};
                reader.Seek(dataChunk.value().offset + track.soundOffset, Common::BinaryFileReader::SeekOrigin::Begin);

                size_t total = 0;
                while (total < track.soundLength) {
                    size_t readLength = std::min<size_t>(buffer.size(), track.soundLength - total);
                    reader.ReadBytes(buffer, readLength);
                    writer.writePCM(buffer.data(), readLength);
                    total += readLength;
                }
                writer.close();
            }

            return true;
        }

    private:
        std::string_view m_InputFile;
        std::string_view m_OutputDir;
    };
}
