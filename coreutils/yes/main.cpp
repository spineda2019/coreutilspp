// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@wpi.edu)

#include <print>
#include <ranges>
#include <string_view>

#include "lib/ArgumentParser.hpp"

int main(int argc, const char **argv) {
    using PositionalArgs =
        coreutils::PositionalArguments<std::string_view,
                                       [](std::string_view v) { return v; }>;
    coreutils::ArgumentParser<"yes", "0.0.1", PositionalArgs> parser{argc,
                                                                     argv};
    parser.ParseArgsOrExit();
    decltype(auto) pos_args{parser.get<PositionalArgs>().value};
    if (argc > 2) {
        std::println("Usage: yes [string]");
        return 1;
    }

    std::string_view to_print{"y"};

    if (pos_args.size()) {
        // decltype(auto) joined{std::ranges::join_view{pos_args}};
        // to_print = std::string_view{joined.begin(), joined.end()};
    }

    while (true) {
        std::println("{}", to_print);
    }

    return 0;
}
