// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

#ifndef LIB_ARGUMENTPARSER_HPP_
#define LIB_ARGUMENTPARSER_HPP_

#include <array>
#include <cstddef>

namespace coreutils {

namespace util {}  // namespace util

template <std::size_t Length>
struct ProgramName final {
    /// Cannot be marked as explicit, as this is intended to be used as a
    /// template argument in ArgumentParser, so this allows implicit conversions
    /// (e.g. ArgumentParser<"FooProgram">)
    consteval ProgramName(const char (&name)[Length]) {
        for (std::size_t i{0}; i < Length; ++i) {
            name_[i] = name[i];
        }
    }

    constexpr bool operator==(const ProgramName&) const = default;
    constexpr auto operator<=>(const ProgramName&) const = default;

    std::array<char, Length> name_;
};

template <std::size_t Length>
ProgramName(const char (&)[Length]) -> ProgramName<Length>;

template <ProgramName Name>
class ArgumentParser final {
 public:
    constexpr explicit ArgumentParser(int argc, const char** argv) {
        (void)argc;
        (void)argv;
    }
};
}  // namespace coreutils

#endif  // LIB_ARGUMENTPARSER_HPP_
