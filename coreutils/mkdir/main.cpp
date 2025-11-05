///
///  @file main.cpp
///  @brief Create directries if they don't exist
///
///  Copyright (C) 2025  Sebastian Pineda (spineda.wpi.alum@gmail.com)
///
///  This program is free software; you can redistribute it and/or modify
///  it under the terms of the GNU General Public License as published by
///  the Free Software Foundation; either version 2 of the License, or
///  (at your option) any later version.
///
///  This program is distributed in the hope that it will be useful,
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///  GNU General Public License for more details.
///
///  You should have received a copy of the GNU General Public License along
///  with this program. If not, see <https://www.gnu.org/licenses/>
///

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
    using Parents = coreutils::BooleanArgument<"-p", "--parents">;

    coreutils::ArgumentParser<Mkdir, PosArgs, Parents> parser{argc, argv};
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
        const std::string& str{name.string()};
        std::println("\targ {}: {}", count, str);
    }
}
