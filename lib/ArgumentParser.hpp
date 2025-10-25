// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

#ifndef LIB_ARGUMENTPARSER_HPP_
#define LIB_ARGUMENTPARSER_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace core {

namespace flags {
struct Help {};
struct Version {};
}  // namespace flags

enum class nargs : std::uint8_t {};

template <class T, const char*... Names>
class Argument final {};

template <>
class Argument<flags::Help> final {};

template <>
class Argument<flags::Version> final {};

template <class T>
class Argument<T> final {
    static_assert(false, "Argument with a type must have at least one Name");
};

template <Argument... Args>
class ArgumentParser final {
 public:
    void ParseArgs(std::span<const char*> args) const {
        for (std::string_view arg : args) {
            (void)arg;
        }
    }

 private:
};
}  // namespace core

#endif  // LIB_ARGUMENTPARSER_HPP_
