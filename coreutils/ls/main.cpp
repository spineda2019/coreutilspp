///
///  @file main.cpp
///  @brief List files in a given directory (or directories)
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

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <print>
#include <string>
#include <string_view>
#include <vector>

#include "lib/ArgumentParser.hpp"

int main(int argc, const char** argv) {
    using Ls = coreutils::ProgramInfo<
        "ls", "0.0.1", "ls [OPTION]... [FILE]...",
        "List information about the FILEs (the current directory by default).\n"
        "Sort entries alphabetically if none of -cftuvSUX nor --sort is "
        "specified.">;
    using PosArgs =
        coreutils::PositionalArguments<std::filesystem::path,
                                       [](std::string_view arg) {
                                           return std::filesystem::path{arg};
                                       }>;
    coreutils::ArgumentParser<Ls, PosArgs> parser{argc, argv};
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

    namespace fs = std::filesystem;
    using Path = fs::path;
    const std::vector<Path>& dirs{parser.get<PosArgs>().value};
    const auto display = [](const Path& entry) {
        if (auto name{entry.filename().string()}; !name.starts_with('.')) {
            std::print("{} ", entry.filename().string());
        }
    };

    switch (dirs.size()) {
        case 0:
            std::ranges::for_each(fs::directory_iterator{fs::current_path()},
                                  display);
            std::println();
            break;
        case 1:
            if (Path target{dirs.front()}; fs::is_directory(target)) {
                std::ranges::for_each(fs::directory_iterator{target}, display);
                std::println();
            } else {
                std::println("{}", target.string());
            }
            break;
        default:
            for (auto it{dirs.cbegin()}; it < dirs.cend(); ++it) {
                std::println("{}:", it->string());
                if (fs::is_directory(*it)) {
                    std::ranges::for_each(fs::directory_iterator{*it}, display);
                    std::println();
                } else {
                    std::println("{}", it->string());
                }
                std::println();
            }
            break;
    }
    return 0;
}
