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
    using Mkdir = coreutils::ProgramInfo<
        "mkdir", "0.0.1", "Usage: mkdir [OPTION]... DIRECTORY...",
        "Create the DIRECTORY(ies), if they do not already exist.">;
    using PosArgs =
        coreutils::PositionalArguments<std::filesystem::path,
                                       [](std::string_view arg) {
                                           return std::filesystem::path{arg};
                                       }>;

    coreutils::ArgumentParser<Mkdir, PosArgs> parser{argc, argv};
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
    std::println("Positional arguments were:");
    std::size_t count{0};
    for (const std::filesystem::path& name : parser.get<PosArgs>().value) {
        ++count;
        std::println("\targ {}: {}", count, name.string());
    }
}
