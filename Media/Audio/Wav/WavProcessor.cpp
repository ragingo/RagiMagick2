#include "WavProcessor.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <print>
#include <string>
#include <utility>
#include <vector>
#include "Audio/Wav/Wav.h"
#include "Audio/Wav/WavParser.h"
#include "Audio/Wav/WavWriter.h"
#include "Common/BinaryFileReader.h"

namespace RagiMagick2::Audio::Wav
{
    WavProcessor::WavProcessor(std::string_view inputFile, std::string_view outputFile) noexcept
        : m_InputFile(inputFile)
        , m_OutputFile(outputFile)
        , m_Speed(1.0)
        , m_Channels(std::nullopt)
    {
    }

    bool WavProcessor::execute() noexcept
    {
        WavParser parser(m_InputFile);
        if (!parser.parse()) {
            std::println(stderr, "WAVファイルのパースに失敗しました: {}", m_InputFile);
            return false;
        }

        if (!parser.getFormatChunk().has_value() || !parser.getDataChunk().has_value()) {
            std::println(stderr, "ファイル内のWAVチャンク情報が無効です: {}", m_InputFile);
            return false;
        }

        Common::BinaryFileReader reader(m_InputFile, false);
        if (!reader.open()) {
            std::println(stderr, "入力ファイルを開けませんでした: {}", m_InputFile);
            return false;
        }

        const auto& inputFormatChunk = parser.getFormatChunk().value();
        const auto& inputDataChunk = parser.getDataChunk().value();
        auto outputFormatChunk = inputFormatChunk;

        updateChannels(inputFormatChunk, outputFormatChunk);

        updateSpeed(inputFormatChunk, outputFormatChunk);

        // 出力データ長を計算
        // サンプル数は同じままだが、ブロックサイズが変わる可能性がある (モノラル/ステレオ)
        uint32_t inputBlockSize = inputFormatChunk.blockSize;
        uint32_t outputBlockSize = outputFormatChunk.blockSize;
        uint32_t sampleCount = inputDataChunk.header.length / inputBlockSize;
        uint32_t outputDataLength = sampleCount * outputBlockSize;

        WavWriter writer{ std::string(m_OutputFile) };
        if (!writer.open()) {
            std::println(stderr, "出力ファイルを開けませんでした: {}", m_OutputFile);
            return false;
        }

        writer.writeRiffChunk(outputDataLength);
        writer.writeFormatChunk(outputFormatChunk);
        writer.writeDataChunkHeader(outputDataLength);

        reader.Seek(inputDataChunk.offset, Common::BinaryFileReader::SeekOrigin::Begin);

        const uint32_t samplesPerBuffer = 1024;
        std::vector<uint8_t> inBuf(inputBlockSize * samplesPerBuffer);
        std::vector<uint8_t> outBuf;

        for (uint32_t processedSamples = 0; processedSamples < sampleCount;) {
            uint32_t targetSampleCount = std::min(samplesPerBuffer, sampleCount - processedSamples);
            uint32_t readBytes = targetSampleCount * inputBlockSize;
            inBuf.resize(readBytes);

            if (reader.ReadBytes(inBuf) != readBytes) {
                std::println(stderr, "エラー: 入力ファイルから期待されるバイト数を読み込めませんでした。");
                writer.close();
                return false;
            }

            const uint8_t* pcmData = inBuf.data();
            uint32_t pcmDataSize = readBytes;

            // チャンネル変換
            if (inputFormatChunk.channels != outputFormatChunk.channels) {
                convertChannels(inputFormatChunk, outputFormatChunk, inBuf, outBuf, targetSampleCount);
                if (!outBuf.empty()) {
                    pcmData = outBuf.data();
                    pcmDataSize = targetSampleCount * outputBlockSize;
                }
                else {
                    std::println(
                        stderr,
                        "サポートされていないチャンネル変換なので無視します。入力: {}, 出力: {}",
                        std::to_underlying(inputFormatChunk.channels),
                        std::to_underlying(outputFormatChunk.channels)
                    );
                }
            }
            else {
                pcmData = inBuf.data();
                pcmDataSize = targetSampleCount * outputBlockSize;
            }

            writer.writePCM(const_cast<uint8_t*>(pcmData), pcmDataSize);
            processedSamples += targetSampleCount;
        }

        writer.close();
        return true;
    }

    void WavProcessor::updateChannels(const FormatChunk& inputFormatChunk, FormatChunk& outputFormatChunk) noexcept
    {
        if (m_Channels.has_value() && m_Channels.value() != inputFormatChunk.channels) {
            switch (m_Channels.value()) {
            case Channel::MONO:
                outputFormatChunk.channels = Channel::MONO;
                outputFormatChunk.blockSize = (inputFormatChunk.bitsPerSample / 8) * 1;
                break;
            case Channel::STEREO:
                outputFormatChunk.channels = Channel::STEREO;
                outputFormatChunk.blockSize = (inputFormatChunk.bitsPerSample / 8) * 2;
                break;
            }
            outputFormatChunk.bytesPerSec = outputFormatChunk.samplingFreq * outputFormatChunk.blockSize;
        }
    }

    void WavProcessor::updateSpeed(const FormatChunk& inputFormatChunk, FormatChunk& outputFormatChunk) noexcept
    {
        if (m_Speed != 1.0) {
            outputFormatChunk.samplingFreq = static_cast<uint32_t>(inputFormatChunk.samplingFreq * m_Speed);
            outputFormatChunk.bytesPerSec = outputFormatChunk.samplingFreq * outputFormatChunk.blockSize;
        }
    }

    void WavProcessor::convertChannels(
        const FormatChunk& inputFormatChunk,
        const FormatChunk& outputFormatChunk,
        const std::vector<uint8_t>& inBuf,
        std::vector<uint8_t>& outBuf,
        uint32_t sampleCount
    ) noexcept
    {
        outBuf.resize(sampleCount * outputFormatChunk.blockSize);

        if (inputFormatChunk.channels == Channel::STEREO && outputFormatChunk.channels == Channel::MONO) {
            if (inputFormatChunk.bitsPerSample == 16) {
                convertStereoToMono<int16_t>(inBuf, outBuf, sampleCount);
            }
            else if (inputFormatChunk.bitsPerSample == 8) {
                convertStereoToMono<uint8_t>(inBuf, outBuf, sampleCount);
            }
        }
        else if (inputFormatChunk.channels == Channel::MONO && outputFormatChunk.channels == Channel::STEREO) {
            if (inputFormatChunk.bitsPerSample == 16) {
                convertMonoToStereo<int16_t>(inBuf, outBuf, sampleCount);
            }
            else if (inputFormatChunk.bitsPerSample == 8) {
                convertMonoToStereo<uint8_t>(inBuf, outBuf, sampleCount);
            }
        }
        else {
            outBuf.clear();
        }
    }

    template <typename T>
    void WavProcessor::convertStereoToMono(
        const std::vector<uint8_t>& inBuf,
        std::vector<uint8_t>& outBuf,
        uint32_t sampleCount
    ) noexcept
    {
        auto* pcmIn = reinterpret_cast<const T*>(inBuf.data());
        auto* pcmOut = reinterpret_cast<T*>(outBuf.data());
        for (uint32_t sample = 0; sample < sampleCount; ++sample) {
            int32_t left = pcmIn[sample * 2];
            int32_t right = pcmIn[sample * 2 + 1];
            pcmOut[sample] = static_cast<T>((left + right) / 2);
        }
    }

    template <typename T>
    void WavProcessor::convertMonoToStereo(
        const std::vector<uint8_t>& inBuf,
        std::vector<uint8_t>& outBuf,
        uint32_t sampleCount
    ) noexcept
    {
        auto* pcmIn = reinterpret_cast<const T*>(inBuf.data());
        auto* pcmOut = reinterpret_cast<T*>(outBuf.data());
        for (uint32_t sample = 0; sample < sampleCount; ++sample) {
            T mono = pcmIn[sample];
            pcmOut[sample * 2] = mono;     // Left
            pcmOut[sample * 2 + 1] = mono; // Right
        }
    }

} // namespace RagiMagick2::Audio::Wav
