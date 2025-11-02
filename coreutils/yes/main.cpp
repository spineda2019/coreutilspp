// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@wpi.edu)

#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "lib/ArgumentParser.hpp"

int main(int argc, const char** argv) {
    using Yes = coreutils::ProgramInfo<
        "yes", "0.0.1", "Usage: yes [STRING]...",
        "Repeatedly output a line with all specified STRING(s), or 'y'.">;
    using PositionalArgs =
        coreutils::PositionalArguments<std::string_view,
                                       [](std::string_view v) { return v; }>;
    coreutils::ArgumentParser<Yes, PositionalArgs> parser{argc, argv};
    parser.ParseArgsOrExit();

    const std::vector<std::string_view>& pos_args{
        parser.get<PositionalArgs>().value};

    std::string to_print{};

    if (std::vector<std::string_view>::size_type size{pos_args.size()}; size) {
        for (const std::string_view& v :
             std::span{pos_args.begin(), size - 1}) {
            to_print.append_range(v);
            to_print.push_back(' ');
        }
        to_print.append_range(pos_args.back());
    } else {
        to_print.push_back('y');
    }

    while (true) {
        std::println("{}", to_print);
    }

    return 0;
}
