// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

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
