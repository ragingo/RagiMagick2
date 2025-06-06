#include <algorithm>
#include <cstdlib>
#include <functional>
#include <print>
#include <string_view>
#include <vector>
#include <nameof.hpp>
#include "CommandLine/CommandLine.h"

#if _WIN32
#include <Windows.h>
#endif

class ScopeGuard
{
public:
    explicit ScopeGuard(std::function<void()> onExit)
        : m_OnExit(onExit)
        , m_IsActive(true)
    {
    }

    ~ScopeGuard()
    {
        if (m_IsActive) {
            m_OnExit();
        }
    }
    void dismiss()
    {
        m_IsActive = false;
    }

private:
    std::function<void()> m_OnExit;
    bool m_IsActive;
};

bool validateCPU();

int main(int argc, char** argv)
{
#if _WIN32
    UINT oldCodePage = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
    ScopeGuard scopeGuard([oldCodePage] {
        SetConsoleOutputCP(oldCodePage);
    });
#endif

    std::vector<std::string_view> args(argv + 1, argv + argc);

    if (args.empty()) {
        std::println("Usage: ConsoleApp <command> <subcommand> [options]");
        return EXIT_FAILURE;
    }

    if (!validateCPU()) {
        return EXIT_FAILURE;
    }

    CommandLine commandLine(args);
    if (!commandLine.parse()) {
        return EXIT_FAILURE;
    }
    if (!commandLine.execute()) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#include "Common/CPU.h"

bool validateCPU()
{
    using namespace RagiMagick2::Common;

    std::println("CPU vendor ID: {}", cpuVendorID());

    auto features = cpuAavailableFeatures();
    for (auto feature : features) {
        std::println("CPU feature: {}", NAMEOF_ENUM(feature));
    }

    if (!std::ranges::contains(features, CPUFeature::AVX2)) {
        std::println("AVX2 is required.");
        return false;
    }

    return true;
}
