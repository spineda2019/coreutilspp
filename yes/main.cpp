// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@wpi.edu)

#include <print>
#include <span>
#include <string_view>

#include "lib/ArgumentParser.hpp"

int main(int argc, const char **argv) {
    static constexpr const char h[]{"-h"};
    static constexpr const char d[]{"-d"};

    using Help = core::BooleanFlag<h>;
    using Dir = core::Flag<d, std::string_view, 1>;

    core::ArgumentParser<Help, Dir> parser{};
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
