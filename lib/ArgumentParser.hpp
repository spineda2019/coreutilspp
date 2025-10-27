// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

#ifndef LIB_ARGUMENTPARSER_HPP_
#define LIB_ARGUMENTPARSER_HPP_

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <print>
#include <span>
#include <string_view>

namespace coreutils {

namespace util {

template <std::size_t Length>
struct ComptimeString final {
    /// Cannot be marked as explicit, as this is intended to be used as a
    /// template argument in ArgumentParser, so this allows implicit conversions
    /// (e.g. ArgumentParser<"FooProgram">)
    consteval ComptimeString(const char (&name)[Length]) {
        // don't include null terminator. That's C crap.
        std::copy(name, name + Length - 1, name_.begin());
    }

    constexpr bool operator==(const ComptimeString&) const = default;
    constexpr auto operator<=>(const ComptimeString&) const = default;

    consteval std::string_view PrintableView() const {
        return std::string_view{name_.begin(), name_.size()};
    }

    std::array<char, Length - 1> name_;
};

template <util::ComptimeString Name>
static consteval std::string_view CreateHelpView() {
    if constexpr (!Name.name_.size()) {
        static constexpr std::string_view pos{"Positional Arguments..."};
        return pos;
    } else {
        return Name.PrintableView();
    }
}

}  // namespace util

template <class T>
concept Arg = requires(T arg) {
    std::same_as<std::string_view, decltype(T::help_view_)>;
};

enum struct NArgs : std::uint8_t {
    None,  // e.g. --verbose
    One,   // e.g. --directory foo/bar
    Many,  // e.g. --names bob sally mary ...
};

template <util::ComptimeString Name, class T, NArgs N>
struct Argument final {
    static inline constexpr std::string_view help_view_{
        util::CreateHelpView<Name>()};

    static inline constexpr void TryParseValue(std::string_view arg) {
        if (arg == Name.PrintableView()) {
            std::println("I am {} and I got ", Name.PrintableView());
        }
    }

    static inline constexpr void TryParseFlag(std::string_view arg) {
        if (arg == Name.PrintableView()) {
            std::println("Arg Match: {}", arg);
            std::println("Name.len: {}", Name.PrintableView().size());
            std::println("arg.len: {}", arg.size());
            std::println();
        }
    }

 private:
    enum class ParseState : std::uint8_t {
        Start,
        End,
    };
};

template <util::ComptimeString Name, util::ComptimeString Version, Arg... Args>
class ArgumentParser final {
 public:
    constexpr explicit ArgumentParser(int argc, const char** argv)
        : args_{argv + 1, argv + argc} {}

    constexpr auto ParseArgsOrExit() {
        for (std::string_view arg : args_) {
            if (arg == "-v" || arg == "--version") {
                PrintVersion();
            } else if (arg == "-h" || arg == "--help") {
                PrintHelp();
            } else {
                if (arg.starts_with('-')) {
                    (Args::TryParseFlag(arg), ...);
                } else {
                    (Args::TryParseValue(arg), ...);
                }
            }
        }
    }

    [[noreturn]]
    constexpr void PrintVersion() const {
        std::println("{} version {}\n\n{}", Name.PrintableView(),
                     Version.PrintableView(), license_info_);
        std::exit(0);
    }

    [[noreturn]]
    constexpr void PrintHelp() const {
        std::println("Usage: {} [OPTIONS]...\n", Name.PrintableView());
        (std::println("\t{}", Args::help_view_), ...);
        std::exit(0);
    }

 private:
    // since argc and argv should be valid for the lifetime of the main
    // function, storing this should be safe, as the lifetime of this object
    // should logically always be less than the main function.
    std::span<const char*> args_{};

    static constexpr std::string_view license_info_{"License info TBD."};
};
}  // namespace coreutils

#endif  // LIB_ARGUMENTPARSER_HPP_
