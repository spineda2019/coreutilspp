///
///  @file main.cpp
///  @brief Print supplied strings to standard output
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

#include <print>
#include <span>

int main(int argc, const char** argv) {
    std::span<const char*> args{argv + 1, argv + argc};

    if (args.size()) {
        const auto last{args.back()};
        for (const char* arg : args) {
            if (arg != last) {
                std::print("{} ", arg);
            } else {
                std::println("{}", arg);
            }
        }
    } else {
        std::println();
    }

    return 0;
}
