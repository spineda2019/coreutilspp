// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

#ifndef LIB_DETAIL_ARGUMENTPARSER_HPP_
#define LIB_DETAIL_ARGUMENTPARSER_HPP_

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>

namespace coreutils {
namespace detail {

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

/// Represents a possible state of an Argument As it goes along the parsing
/// process. State transitions should be 100% deterministic and proceed
/// with each token consumed from the command line. This should theortically
/// be modelable with a DFA:
///
enum class ParseState : std::uint8_t {
    Start,
    Seeking,
    End,
};

enum struct NArgs : std::uint8_t {
    None,  // e.g. --verbose
    One,   // e.g. --directory foo/bar
    Many,  // e.g. --names bob sally mary ...
};

template <ComptimeString PrimaryName, ComptimeString... Aliases>
struct ArgumentBase {
    static_assert(PrimaryName.PrintableView().starts_with("-"),
                  "Primary Argument name must start with a -");

    static inline constexpr std::array<std::string_view, sizeof...(Aliases) + 1>
        names_{PrimaryName.PrintableView(), Aliases.PrintableView()...};

    static inline constexpr std::string_view help_view_{
        PrimaryName.PrintableView()};

 protected:
    ParseState state_{ParseState::Start};
};

template <>
struct ArgumentBase<""> {
    static inline constexpr std::array<std::string_view, 0> names_{};

    static inline constexpr std::string_view help_view_{
        "Positional Arguments..."};

 protected:
    ParseState state_{ParseState::Start};
};

template <class ParseType, NArgs N, auto Converter, ComptimeString... Names>
struct Argument final {
    static_assert(false, "Use a specialized version of this struct");
};

/// Specialization to specify an argument that can receive one or more
/// args
template <class T, auto Converter, ComptimeString... Names>
    requires std::regular_invocable<decltype(Converter), std::string_view>
struct Argument<T, NArgs::Many, Converter, Names...> : ArgumentBase<Names...> {
    static_assert(!std::is_same_v<void, T>,
                  "Flag arguments cannot be of type void");

    constexpr void TryParseValue(std::string_view arg) {
        switch (ArgumentBase<Names...>::state_) {
            case ParseState::Start:
            case ParseState::End:
                // ignore
                break;
            case ParseState::Seeking:
                value.emplace_back(std::invoke(Converter, arg));
                break;
        }
    }
    constexpr void TryParseFlag(std::string_view arg) {
        bool is_this{std::ranges::any_of(
            ArgumentBase<Names...>::names_,
            [arg](std::string_view name) { return name == arg; })};

        switch (ArgumentBase<Names...>::state_) {
            case ParseState::Start:
                if (is_this) {
                    ArgumentBase<Names...>::state_ = ParseState::Seeking;
                }
                break;
            case ParseState::Seeking:
                if (is_this) {
                    throw std::runtime_error{
                        std::format("ERROR! Duplicate option: {}", arg)};
                } else if (!value.size()) {
                    throw std::runtime_error{std::format(
                        "ERROR! Do not specify {} and supply no arguments",
                        arg)};
                } else {
                    ArgumentBase<Names...>::state_ = ParseState::End;
                }
                break;
            case ParseState::End:
                if (is_this) {
                    throw std::runtime_error{
                        std::format("ERROR! Duplicate option: {}", arg)};
                }
                break;
        }
    }

    // TODO(SEP): maybe take a template-template parameter to not force vector?
    std::vector<T> value{};
};

template <class T, auto Converter>
struct Argument<T, NArgs::Many, Converter, ""> : ArgumentBase<""> {
    static_assert(!std::is_same_v<void, T>,
                  "Positional arguments cannot be of type void");

    constexpr void TryParseValue(std::string_view arg) {
        switch (this->state_) {
            case ParseState::Start:
                ArgumentBase<"">::state_ = ParseState::Seeking;
                // NOTE: fallthrough
            case ParseState::Seeking:
                value.emplace_back(std::invoke(Converter, arg));
                break;
            case ParseState::End:
                break;
        }
    }
    constexpr void TryParseFlag(std::string_view _) {
        ArgumentBase<"">::state_ = ParseState::End;
    }

    // TODO(SEP): maybe take a template-template parameter to not force vector?
    std::vector<T> value{};
};

template <class T, auto Converter, ComptimeString... Names>
struct Argument<T, NArgs::None, Converter, Names...> : ArgumentBase<Names...> {
    static_assert(std::is_same_v<void, T>,
                  "A flag returning no values cannot have a non-void type");

    constexpr void TryParseValue(std::string_view _) {}
    constexpr void TryParseFlag(std::string_view arg) {
        bool is_this{std::ranges::any_of(
            ArgumentBase<Names...>::names_,
            [arg](std::string_view name) { return name == arg; })};

        if (is_this) {
            switch (this->state_) {
                case ParseState::Start:
                    value = true;
                    this->state_ = ParseState::End;
                    break;
                case ParseState::Seeking:
                case ParseState::End:
                    throw std::runtime_error{
                        std::format("ERROR! Duplicate option: {}", arg)};
                    break;
            }
        }
    }

    bool value{};
};

template <class T, auto Converter, ComptimeString... Names>
    requires std::regular_invocable<decltype(Converter), std::string_view>
struct Argument<T, NArgs::One, Converter, Names...> : ArgumentBase<Names...> {
    static_assert(!std::is_same_v<void, T>,
                  "Flag argument cannot be of type void");

    constexpr void TryParseValue(std::string_view arg) {
        switch (this->state_) {
            case ParseState::Start:
            case ParseState::End:
                break;
            case ParseState::Seeking:
                value = std::invoke(Converter, arg);
                ArgumentBase<Names...>::state_ = ParseState::End;
                break;
        }
    }
    constexpr void TryParseFlag(std::string_view arg) {
        bool is_this{std::ranges::any_of(
            ArgumentBase<Names...>::names_,
            [arg](std::string_view name) { return name == arg; })};

        switch (ArgumentBase<Names...>::state_) {
            case ParseState::Start:
                if (is_this) {
                    this->state_ = ParseState::Seeking;
                }
                break;
            case ParseState::Seeking:
            case ParseState::End:
                if (is_this) {
                    throw std::runtime_error{std::format(
                        "ERROR: unexpected repeated flag: {}", arg)};
                }
                break;
        }
    }

    T value{};
};
}  // namespace detail
}  // namespace coreutils

#endif  // LIB_DETAIL_ARGUMENTPARSER_HPP_
