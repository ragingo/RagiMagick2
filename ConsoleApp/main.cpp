#include <algorithm>
#include <cstdlib>
#include <print>
#include <string_view>
#include <vector>
#include <nameof.hpp>
#include "CommandLine/CommandLine.h"

#ifdef _WIN32
#include <windows.h>
#endif

bool validateCPU();

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
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