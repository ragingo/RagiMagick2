#include "WavParser.h"
#include <algorithm>
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
#include "WavSplitter.h"

namespace RagiMagick2::Audio::Wav
{
    WavParser::WavParser(std::string_view wavFileName) noexcept
        : m_WavFileName(std::string(wavFileName))
        , m_Reader(Common::BinaryFileReader(m_WavFileName, false))
    {
    }

    bool WavParser::parse() noexcept
    {
        // .cue 存在チェック
        auto wavFilePath = std::filesystem::path(m_WavFileName);
        auto cueFilePath = wavFilePath.replace_extension("cue");
        if (std::filesystem::exists(cueFilePath)) {
            m_CueFileName = cueFilePath.string();
        }

        if (!m_Reader.open()) {
            return false;
        }

        if (!parseRiffContainer()) {
            return false;
        }

        if (!m_CueFileName.empty()) {
            CD::CueParser parser(m_CueFileName);
            m_Cue = parser.parse();
            parseMultiTrackWav();
        }

        return true;
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

        tracks.swap(m_Tracks);
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
