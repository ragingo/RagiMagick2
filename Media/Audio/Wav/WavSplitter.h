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
#include "WavWriter.h"

namespace RagiMagick2::Audio::Wav
{
    class WavSplitter final
    {
    public:
        WavSplitter() {}

        void run(
            std::string_view outputPath,
            const std::vector<Track>& tracks,
            Common::BinaryFileReader& reader,
            const FormatChunk& formatChunk,
            const DataChunk& dataChunk
        ) {
            for (const auto& track : tracks) {
                if (reader.isEOF()) {
                    break;
                }
                if (track.soundOffset >= dataChunk.header.length) {
                    std::println(stderr, "トラック {} のオフセットがデータ長を超えている", track.id);
                    continue;
                }
                reader.Seek(track.soundOffset, Common::BinaryFileReader::SeekOrigin::Begin);
                std::string fileName = std::format("{}/{:02}_{}.wav", outputPath, track.id, track.title);

                WavWriter writer(fileName);
                if (!writer.open()) {
                    std::println(stderr, "{} のオープンに失敗した", fileName);
                    continue;
                }
                writer.writeRiffChunk(track.soundLength);
                writer.writeFormatChunk(formatChunk);
                writer.writeDataChunkHeader(track.soundLength);

                std::array<uint8_t, 1024> buffer{};
                reader.Seek(dataChunk.offset + track.soundOffset, Common::BinaryFileReader::SeekOrigin::Begin);

                size_t total = 0;
                while (total < track.soundLength) {
                    size_t readLength = std::min<size_t>(buffer.size(), track.soundLength - total);
                    reader.ReadBytes(buffer, readLength);
                    writer.writePCM(buffer.data(), readLength);
                    total += readLength;
                }
                writer.close();
            }
        }
    };
}
