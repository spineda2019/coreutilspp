// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@wpi.edu)

#include <print>
#include <span>

#include "lib/ArgumentParser.hpp"

int main(int argc, const char **argv) {
    constexpr core::Argument<core::flags::Help> help{};
    constexpr core::Argument<core::flags::Version> version{};

    core::ArgumentParser<help, version> parser{};

    parser.ParseArgs(std::span<const char *>{argv + 1, argv + argc});

    if (argc > 2) {
        std::println("Usage: yes [string]");
        return 1;
    }

    const char *to_print{"y"};

    switch (argc) {
        case 1:
            break;
        case 2:
            to_print = argv[1];
            break;
        default:
            std::println("Usage: yes [string]");
            return 1;
    }

    while (true) {
        std::println("{}", to_print);
    }

    return 0;
}
