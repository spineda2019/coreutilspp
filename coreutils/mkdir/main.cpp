// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <print>
#include <string>
#include <string_view>

#include "lib/ArgumentParser.hpp"

int main(int argc, const char** argv) {
    using Verbose = coreutils::BooleanArgument<"-V">;
    using Test =
        coreutils::SingleValueArgument<"-t", int, [](std::string_view view) {
            return std::stoi(std::string{view});
        }>;
    using Dirs =
        coreutils::PositionalArguments<std::filesystem::path,
                                       [](std::string_view view) {
                                           return std::filesystem::path{view};
                                       }>;
    using Names = coreutils::MultiValueArgument<"-n", std::string,
                                                [](std::string_view view) {
                                                    return std::string{view};
                                                }>;

    coreutils::ArgumentParser<"mkdir", "0.0.1", Verbose, Dirs, Test, Names>
        parser{argc, argv};
    try {
        parser.ParseArgsOrExit();
    } catch (const std::exception& ex) {
        std::println(std::cerr, "Error occured while parsing arguments: {}",
                     ex.what());
        return 1;
    } catch (...) {
        std::println(std::cerr, "Unrecognized error occurred.");
        return 1;
    }
    std::println("-V was: {}", Verbose::value);
    std::println("-t was: {}", Test::value);
    std::println("-n was:");
    std::size_t count{0};
    for (std::string_view name : Names::value) {
        ++count;
        std::println("\targ {}: {}", count, name);
    }
    std::println("Positional arguments were:");
    count = 0;
    for (const std::filesystem::path& name : Dirs::value) {
        ++count;
        std::println("\targ {}: {}", count, name.c_str());
    }
}
