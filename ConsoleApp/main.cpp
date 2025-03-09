#include <cstdlib>
#include <print>
#include <string_view>
#include <vector>
#include "CommandLine/CommandLine.h"

int main(int argc, char** argv)
{
    std::vector<std::string_view> args(argv + 1, argv + argc);

    if (args.empty()) {
        std::println("Usage: ConsoleApp <command> <subcommand> [options]");
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
