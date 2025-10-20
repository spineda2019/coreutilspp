// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

#ifndef LIB_ARGUMENTPARSER_HPP_
#define LIB_ARGUMENTPARSER_HPP_

#include <cstddef>
#include <span>

namespace core {

/// e.g <"-d", 1, std::filesystem::path>
template <const char* Id, class T, std::size_t NumValues>
class Flag {};

/// e.g <"-d", std::filesystem::path>
template <const char* Id, class T>
class Flag<Id, T, 1> {};

/// e.g <"-d", std::filesystem::path>
template <const char* Id>
class Flag<Id, void, 0> {};

template <const char* Id>
using BooleanFlag = Flag<Id, void, 0>;

namespace util {
template <class Concrete,
          template <const char*, class, std::size_t> class Template>
inline constexpr bool is_instance_of_template{false};

template <const char* Id, class T, std::size_t NumValues,
          template <const char*, class, std::size_t> class Template>
inline constexpr bool
    is_instance_of_template<Template<Id, T, NumValues>, Template>{true};

}  // namespace util

template <class T>
concept FlagType = util::is_instance_of_template<T, Flag>;

template <FlagType... FlagTypes>
class ArgumentParser final {
 public:
    void ParseArgs(std::span<const char*> args) const { (void)args; }

 private:
};
}  // namespace core

#endif  // LIB_ARGUMENTPARSER_HPP_
