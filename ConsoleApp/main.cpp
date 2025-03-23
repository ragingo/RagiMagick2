#include <cstdlib>
#include <print>
#include <string_view>
#include <vector>
#include <nameof.hpp>
#include "CommandLine/CommandLine.h"

bool validateCPU();

int main(int argc, char** argv)
{
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

    if (features.end() == std::find(features.begin(), features.end(), CPUFeature::AVX2)) {
        std::println("AVX2 is required.");
        return false;
    }

    return true;
}