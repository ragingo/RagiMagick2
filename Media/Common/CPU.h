#pragma once
#include <intrin.h>
#include <string>
#include <vector>

namespace RagiMagick2::Common
{
    struct CPUID
    {
        int eax;
        int ebx;
        int ecx;
        int edx;
    };

    enum class CPUFeature
    {
        // MMX
        MMX = 0,
        // SSE
        SSE,
        // SSE2
        SSE2,
        // SSE3
        SSE3,
        // SSSE3
        SSSE3,
        // SSE4.1
        SSE41,
        // SSE4.2
        SSE42,
        // AVX
        AVX,
        // AVX2
        AVX2,
        // AVX512 Foundation
        AVX512F,
        // AVX512 Doubleword and Quadword Instructions
        AVX512DQ,
        // AVX512 Integer Fused Multiply-Add Instructions
        AVX512IFMA,
        // AVX512 Prefetch Instructions
        AVX512PF,
        // AVX512 Exponential and Reciprocal Instructions
        AVX512ER,
        // AVX512 Conflict Detection Instructions
        AVX512CD,
        // AVX512 Byte and Word Instructions
        AVX512BW,
        // AVX512 Vector Length Extensions
        AVX512VL,
        // AVX512 Vector Bit Manipulation Instructions
        AVX512VBMI,
        // AVX512 Vector Bit Manipulation Instructions 2
        AVX512VBMI2,
        // AVX512 Vector Neural Network Instructions
        AVX512VNNI,
        // AVX512 Vector Bitwise Arithmetic Instructions
        AVX512BITALG,
        // AVX512 Vector Population Count Doubleword and Quadword Instructions
        AVX512VPOPCNTDQ
    };

    inline CPUID cpuid(int leaf)
    {
        CPUID id{};
        __cpuidex(reinterpret_cast<int*>(&id), leaf, 0);
        return id;
    }

    inline std::string cpuVendorID()
    {
        auto leaf0 = cpuid(0);
        char vendor[13] = {};
        memcpy(vendor + 0, &leaf0.ebx, 4);
        memcpy(vendor + 4, &leaf0.edx, 4);
        memcpy(vendor + 8, &leaf0.ecx, 4);
        return vendor;
    }

    inline std::vector<CPUFeature> cpuAavailableFeatures()
    {
        using enum RagiMagick2::Common::CPUFeature;
        std::vector<CPUFeature> features;
        auto leaf1 = cpuid(1);
        auto leaf7 = cpuid(7);

        if (leaf1.edx & (1 << 23)) {
            features.emplace_back(MMX);
        }
        if (leaf1.edx & (1 << 25)) {
            features.emplace_back(SSE);
        }
        if (leaf1.edx & (1 << 26)) {
            features.emplace_back(SSE2);
        }
        if (leaf1.ecx & (1 << 0)) {
            features.emplace_back(SSE3);
        }
        if (leaf1.ecx & (1 << 9)) {
            features.emplace_back(SSSE3);
        }
        if (leaf1.ecx & (1 << 19)) {
            features.emplace_back(SSE41);
        }
        if (leaf1.ecx & (1 << 20)) {
            features.emplace_back(SSE42);
        }
        if (leaf1.ecx & (1 << 28)) {
            features.emplace_back(AVX);
        }
        if (leaf7.ebx & (1 << 5)) {
            features.emplace_back(AVX2);
        }
        if (leaf7.ebx & (1 << 16)) {
            features.emplace_back(AVX512F);
        }
        if (leaf7.ebx & (1 << 17)) {
            features.emplace_back(AVX512DQ);
        }
        if (leaf7.ebx & (1 << 21)) {
            features.emplace_back(AVX512IFMA);
        }
        if (leaf7.ebx & (1 << 26)) {
            features.emplace_back(AVX512PF);
        }
        if (leaf7.ebx & (1 << 27)) {
            features.emplace_back(AVX512ER);
        }
        if (leaf7.ebx & (1 << 28)) {
            features.emplace_back(AVX512CD);
        }
        if (leaf7.ebx & (1 << 30)) {
            features.emplace_back(AVX512BW);
        }
        if (leaf7.ebx & (1 << 31)) {
            features.emplace_back(AVX512VL);
        }
        if (leaf7.ecx & (1 << 1)) {
            features.emplace_back(AVX512VBMI);
        }
        if (leaf7.ecx & (1 << 6)) {
            features.emplace_back(AVX512VBMI2);
        }
        if (leaf7.ecx & (1 << 11)) {
            features.emplace_back(AVX512VNNI);
        }
        if (leaf7.ecx & (1 << 12)) {
            features.emplace_back(AVX512BITALG);
        }
        if (leaf7.ecx & (1 << 14)) {
            features.emplace_back(AVX512VPOPCNTDQ);
        }

        return features;
    }
} // namespace RagiMagick2::Common
