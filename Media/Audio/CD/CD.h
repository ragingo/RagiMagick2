#pragma once
#include <cstdint>

namespace RagiMagick2::Audio::CD
{

    // サンプリング周波数
    static constexpr uint32_t SAMPLING_RATE = 44100;
    // チャンネル数
    static constexpr uint32_t CHANNELS = 2;
    // 1サンプルあたりのビット数
    static constexpr uint32_t BITS_PER_SAMPLE = 16;
    // 1フレームあたりのサンプル数
    static constexpr uint32_t FRAMES_PER_SAMPLE = (BITS_PER_SAMPLE / 8) * CHANNELS;
    // 1秒あたりのサンプル数
    static constexpr uint32_t SAMPLES_PER_SECOND = SAMPLING_RATE * CHANNELS;
    // 1秒あたりのバイト数
    static constexpr uint32_t BYTES_PER_SECOND = SAMPLES_PER_SECOND * (BITS_PER_SAMPLE / 8);
    // 1秒あたりのフレーム数
    static constexpr uint32_t FRAMES_PER_SECOND = 75;
    // 1フレームあたりのバイト数
    static constexpr uint32_t BYTES_PER_FRAME = BYTES_PER_SECOND / FRAMES_PER_SECOND;

    // 分、秒、フレーム数から合計フレーム数を取得
    static constexpr uint32_t getFrames(int minutes, int seconds, int frames)
    {
        return (minutes * 60 + seconds) * FRAMES_PER_SECOND + frames;
    }
    static_assert(getFrames(4, 46, 20) == 21470);

    // フレーム数からバイト数を取得
    static constexpr uint32_t getBytesFromFrames(uint32_t frames)
    {
        return frames * BYTES_PER_FRAME;
    }
    static_assert(getBytesFromFrames(75) == BYTES_PER_SECOND);

} // namespace RagiMagick2::Audio::CD
