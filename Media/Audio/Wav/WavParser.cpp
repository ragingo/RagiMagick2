#include "WavParser.h"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <print>
#include <string>
#include <utility>
#include <vector>
#include "Common/BinaryFileReader.h"
#include "Audio/CD/CD.h"
#include "Audio/CD/Cue.h"
#include "Audio/CD/CueParser.h"
#include "Wav.h"
#include "WavWriter.h"

namespace RagiMagick2::Audio::Wav
{
    WavParser::WavParser(std::string_view wavFileName) noexcept
        : m_WavFileName(std::string(wavFileName))
        , m_Reader(Common::BinaryFileReader(m_WavFileName, false))
    {
    }

    void WavParser::parse() noexcept
    {
        // .cue 存在チェック
        auto wavFilePath = std::filesystem::path(m_WavFileName);
        auto cueFilePath = wavFilePath.replace_extension("cue");
        if (std::filesystem::exists(cueFilePath)) {
            m_CueFileName = cueFilePath.string();
        }


        if (!m_Reader.open()) {
            return;
        }

        if (!parseRiffContainer()) {
            return;
        }

        if (!m_CueFileName.empty()) {
            CD::CueParser parser(m_CueFileName);
            m_Cue = parser.parse();
            parseMultiTrackWav();
        }
    }

    bool WavParser::parseRiffContainer() noexcept
    {
        while (!m_Reader.isEOF()) {
            ChunkID chunkID;
            m_Reader.ReadUInt32(chunkID);

            switch (chunkID) {
            case ChunkID::RIFF:
                parseRiffChunk();
                break;
            case ChunkID::FMT:
                parseFormatChunk();
                break;
            case ChunkID::DATA:
                parseDataChunk();
                break;
            }
        }

        if (!m_RiffChunk || !m_FormatChunk || !m_DataChunk) {
            std::println(stderr, "必須チャンク RIFF, fmt, data のいずれかが存在しない");
            return false;
        }

        const auto& riff = *m_RiffChunk;
        const auto& fmt = *m_FormatChunk;
        const bool isWave = riff.fileID == FileID::WAVE;
        const auto blockSize = (fmt.bitsPerSample / 8) * std::to_underlying(fmt.channels);

        if (!isWave) {
            std::println("invalid FileID: 0x{0:X}", std::to_underlying(riff.fileID));
        }

        if (!(isWave && (fmt.bitsPerSample == 8 || fmt.bitsPerSample == 16))) {
            std::println(stderr, "WAVE なのに 1 サンプルあたりのビット数が 8 または 16 ではない");
            return false;
        }

        if (fmt.channels != Channel::MONO && fmt.channels != Channel::STEREO) {
            std::println(stderr, "チャンネル数が 1 または 2 ではない");
            return false;
        }

        if (fmt.blockSize != blockSize) {
            std::println(stderr, "ブロック(1サンプル)のサイズがおかしい");
            return false;
        }

        if (fmt.bytesPerSec != fmt.samplingFreq * blockSize) {
            std::println(stderr, "1秒あたりのサイズがおかしい");
            return false;
        }

        return true;
    }

    void WavParser::parseMultiTrackWav() noexcept
    {
        assert(m_Cue);
        assert(m_DataChunk);
        const auto& cue = *m_Cue;
        const auto& fmt = *m_FormatChunk;
        const auto& data = *m_DataChunk;

        if (fmt.bitsPerSample != CD::BITS_PER_SAMPLE) {
            std::println(stderr, "ビット深度が 16 ではない");
            return;
        }

        if (std::to_underlying(fmt.channels) != CD::CHANNELS) {
            std::println(stderr, "チャンネル数が 2 ではない");
            return;
        }

        if (fmt.samplingFreq != CD::SAMPLING_RATE) {
            std::println(stderr, "サンプリング周波数が 44,100 Hz ではない");
            return;
        }

        if (fmt.bytesPerSec != CD::BYTES_PER_SECOND) {
            std::println(stderr, "1秒あたりのサイズが 176,400 ではない");
            return;
        }

        std::vector<Track> tracks;
        tracks.reserve(cue.tracks.size());

        for (const auto& cueTrack : cue.tracks) {
            if (cueTrack.indices.empty()) {
                std::println(stderr, "トラック {} にインデックスが存在しない", cueTrack.id);
                continue;
            }

            Track track{};
            track.id = cueTrack.id;
            track.title = cueTrack.title;

            auto preGaps = cueTrack.indices | std::views::filter([](const auto& index) {
                return isPreGap(index);
            });
            if (!preGaps.empty()) {
                const auto& preGap = preGaps.front();
                auto frames = CD::getFrames(preGap.minutes, preGap.seconds, preGap.frames);
                track.preGapOffset = CD::getBytesFromFrames(frames);
            }

            const CD::CueTrackIndex* sound = nullptr;
            if (preGaps.empty()) {
                sound = &cueTrack.indices.front();
            }
            else if (cueTrack.indices.size() > 1) {
                sound = &cueTrack.indices[1];
            }
            else {
                continue;
            }
            assert(sound != nullptr);
            auto frames = CD::getFrames(sound->minutes, sound->seconds, sound->frames);
            track.soundOffset = CD::getBytesFromFrames(frames);

            tracks.emplace_back(track);
        }

        // 各トラックの長さを計算
        for (size_t i = 0; i < tracks.size(); ++i) {
            // トラック N の長さは = トラック N+1 の開始オフセット - トラック N の開始オフセット
            if (i < tracks.size() - 1) {
                tracks[i].soundLength = tracks[i + 1].soundOffset - tracks[i].soundOffset;
            }
            else {
                // 最終トラックの長さ = data チャンクの長さ(= 終端) - 最終トラックの開始オフセット
                tracks[i].soundLength = m_DataChunk->header.length - tracks[i].soundOffset;
            }
        }

        m_Reader.clear();
        m_Reader.Seek(data.offset, Common::BinaryFileReader::SeekOrigin::Begin);

        for (const auto& track : tracks) {
            if (m_Reader.isEOF()) {
                break;
            }
            if (track.soundOffset >= data.header.length) {
                std::println(stderr, "トラック {} のオフセットがデータ長を超えている", track.id);
                continue;
            }
            m_Reader.Seek(track.soundOffset, Common::BinaryFileReader::SeekOrigin::Begin);
            std::string fileName = m_WavFileName + "_track_" + std::to_string(track.id) + ".wav";

            WavWriter writer(fileName);
            if (!writer.open()) {
                std::println(stderr, "{} のオープンに失敗した", fileName);
                continue;
            }
            writer.writeRiffChunk(track.soundLength);
            writer.writeFormatChunk(*m_FormatChunk);
            writer.writeDataChunkHeader(track.soundLength);

            std::array<uint8_t, 1024> buffer{};
            m_Reader.Seek((*m_DataChunk).offset + track.soundOffset, Common::BinaryFileReader::SeekOrigin::Begin);

            size_t bytesRead = 0;
            while (bytesRead < track.soundLength) {
                size_t toRead = std::min<size_t>(buffer.size(), track.soundLength - bytesRead);
                m_Reader.ReadBytes(buffer, toRead);
                writer.writePCM(buffer.data(), toRead);
                bytesRead += toRead;
            }
            writer.close();
        }
    }

    void WavParser::parseRiffChunk() noexcept
    {
        RiffChunk riff{};
        m_Reader.ReadUInt32(riff.length);
        m_Reader.ReadUInt32(riff.fileID);
        m_RiffChunk.emplace(riff);
    }

    void WavParser::parseFormatChunk() noexcept
    {
        FormatChunk fmt{};
        m_Reader.ReadUInt32(fmt.length);
        m_Reader.ReadUInt16(fmt.format);
        m_Reader.ReadUInt16(fmt.channels);
        m_Reader.ReadUInt32(fmt.samplingFreq);
        m_Reader.ReadUInt32(fmt.bytesPerSec);
        m_Reader.ReadUInt16(fmt.blockSize);
        m_Reader.ReadUInt16(fmt.bitsPerSample);
        m_FormatChunk.emplace(fmt);
    }

    void WavParser::parseDataChunk() noexcept
    {
        DataChunk data{};
        m_Reader.ReadUInt32(data.header.length);
        data.offset = m_Reader.GetCurrentPosition();
        m_Reader.Seek(data.header.length, Common::BinaryFileReader::SeekOrigin::Current);
        m_Reader.ReadUInt8(data.pad);
        m_DataChunk.emplace(data);
    }
}
