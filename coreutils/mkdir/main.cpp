// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>

#include "lib/ArgumentParser.hpp"

int main(int argc, const char** argv) {
    using Verbose =
        coreutils::Argument<"-V", void, coreutils::NArgs::None, nullptr>;
    auto toInt = [](std::string_view view) {
        return std::stoi(std::string{view});
    };
    using Test = coreutils::Argument<"-t", int, coreutils::NArgs::One, toInt>;
    auto toPath = [](std::string_view view) {
        return std::filesystem::path{view};
    };
    using Dirs = coreutils::Argument<"", std::filesystem::path,
                                     coreutils::NArgs::Many, toPath>;

    coreutils::ArgumentParser<"mkdir", "0.0.1", Verbose, Dirs, Test> parser{
        argc, argv};
    parser.ParseArgsOrExit();
}
