// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

#ifndef LIB_ARGUMENTPARSER_HPP_
#define LIB_ARGUMENTPARSER_HPP_

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <print>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

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
static inline consteval std::string_view CreateHelpView() {
    if constexpr (!Name.name_.size()) {
        static constexpr std::string_view pos{"Positional Arguments..."};
        return pos;
    } else {
        return Name.PrintableView();
    }
}

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

}  // namespace util

template <class T>
concept Arg = requires(T arg) {
    std::same_as<std::string_view, decltype(T::help_view_)>;
    T::value;
};

enum struct NArgs : std::uint8_t {
    None,  // e.g. --verbose
    One,   // e.g. --directory foo/bar
    Many,  // e.g. --names bob sally mary ...
};

template <util::ComptimeString Name>
struct ArgumentBase {
    static inline constexpr std::string_view help_view_{
        util::CreateHelpView<Name>()};

 protected:
    util::ParseState state_{util::ParseState::Start};
};

template <util::ComptimeString Name, class T, NArgs N, auto Converter>
struct Argument final {
    static_assert(false, "Use a specialized version of this struct");
};

/// Specialization to specify an argument that can receive one or more
/// args
template <util::ComptimeString Name, class T, auto Converter>
    requires std::regular_invocable<decltype(Converter), std::string_view>
struct Argument<Name, T, NArgs::Many, Converter> : ArgumentBase<Name> {
    static_assert(!std::is_same_v<void, T>,
                  "Flag arguments cannot be of type void");

    constexpr void TryParseValue(std::string_view arg) {
        switch (ArgumentBase<Name>::state_) {
            case util::ParseState::Start:
            case util::ParseState::End:
                // ignore
                break;
            case util::ParseState::Seeking:
                value.emplace_back(Converter(arg));
                break;
        }
    }
    constexpr void TryParseFlag(std::string_view arg) {
        constexpr std::string_view name{Name.PrintableView()};
        switch (ArgumentBase<Name>::state_) {
            case util::ParseState::Start:
                if (name == arg) {
                    ArgumentBase<Name>::state_ = util::ParseState::Seeking;
                }
                break;
            case util::ParseState::Seeking:
                if (name == arg) {
                    throw std::runtime_error{
                        std::format("ERROR! Duplicate option: {}", name)};
                } else if (!value.size()) {
                    throw std::runtime_error{std::format(
                        "ERROR! Do not specify {} and supply no arguments",
                        name)};
                } else {
                    ArgumentBase<Name>::state_ = util::ParseState::End;
                }
                break;
            case util::ParseState::End:
                if (name == arg) {
                    throw std::runtime_error{
                        std::format("ERROR! Duplicate option: {}", name)};
                }
                break;
        }
    }

    // TODO(SEP): maybe take a template-template parameter to not force vector?
    std::vector<T> value{};
};

template <util::ComptimeString Name, class T, auto Converter>
using MultiValueArgument = Argument<Name, T, NArgs::Many, Converter>;

template <class T, auto Converter>
struct Argument<"", T, NArgs::Many, Converter> : ArgumentBase<""> {
    static_assert(!std::is_same_v<void, T>,
                  "Positional arguments cannot be of type void");

    constexpr void TryParseValue(std::string_view arg) {
        switch (this->state_) {
            case util::ParseState::Start:
                ArgumentBase<"">::state_ = util::ParseState::Seeking;
                // NOTE: fallthrough
            case util::ParseState::Seeking:
                value.emplace_back(Converter(arg));
                break;
            case util::ParseState::End:
                break;
        }
    }
    constexpr void TryParseFlag(std::string_view _) {
        ArgumentBase<"">::state_ = util::ParseState::End;
    }

    // TODO(SEP): maybe take a template-template parameter to not force vector?
    std::vector<T> value{};
};

template <class T, auto Converter>
using PositionalArguments = Argument<"", T, NArgs::Many, Converter>;

template <util::ComptimeString Name, class T, auto Converter>
struct Argument<Name, T, NArgs::None, Converter> : ArgumentBase<Name> {
    static_assert(std::is_same_v<void, T>,
                  "A flag returning no values cannot have a non-void type");

    constexpr void TryParseValue(std::string_view _) {}
    constexpr void TryParseFlag(std::string_view arg) {
        constexpr std::string_view name{Name.PrintableView()};
        if (arg == name) {
            switch (this->state_) {
                case util::ParseState::Start:
                    value = true;
                    this->state_ = util::ParseState::End;
                    break;
                case util::ParseState::Seeking:
                case util::ParseState::End:
                    throw std::runtime_error{
                        std::format("ERROR! Duplicate option: {}", name)};
                    break;
            }
        }
    }

    // TODO(SEP): maybe take a template-template parameter to not force vector?
    bool value{};
};

template <util::ComptimeString Name>
using BooleanArgument = Argument<Name, void, NArgs::None, nullptr>;

template <util::ComptimeString Name, class T, auto Converter>
    requires std::regular_invocable<decltype(Converter), std::string_view>
struct Argument<Name, T, NArgs::One, Converter> : ArgumentBase<Name> {
    static_assert(!std::is_same_v<void, T>,
                  "Flag argument cannot be of type void");

    constexpr void TryParseValue(std::string_view arg) {
        switch (this->state_) {
            case util::ParseState::Start:
            case util::ParseState::End:
                break;
            case util::ParseState::Seeking:
                value = Converter(arg);
                ArgumentBase<Name>::state_ = util::ParseState::End;
                break;
        }
    }
    constexpr void TryParseFlag(std::string_view arg) {
        constexpr auto name{Name.PrintableView()};
        switch (ArgumentBase<Name>::state_) {
            case util::ParseState::Start:
                if (arg == name) {
                    this->state_ = util::ParseState::Seeking;
                }
                break;
            case util::ParseState::Seeking:
            case util::ParseState::End:
                if (arg == name) {
                    throw std::runtime_error{std::format(
                        "ERROR: unexpected repeated flag: {}", name)};
                }
                break;
        }
    }

    // TODO(SEP): maybe take a template-template parameter to not force vector?
    T value{};
};

template <util::ComptimeString Name, class T, auto Converter>
using SingleValueArgument = Argument<Name, T, NArgs::One, Converter>;

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
                    std::apply(
                        [arg](auto&... a) { (a.TryParseFlag(arg), ...); },
                        arg_values_);
                } else {
                    std::apply(
                        [arg](auto&... a) { (a.TryParseValue(arg), ...); },
                        arg_values_);
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

    template <class Out>
    constexpr const Out& get() const {
        return std::get<Out>(arg_values_);
    }

 private:
    // since argc and argv should be valid for the lifetime of the main
    // function, storing this should be safe, as the lifetime of this object
    // should logically always be less than the main function.
    std::span<const char*> args_{};
    std::tuple<Args...> arg_values_{};

    static constexpr std::string_view license_info_{"License info TBD."};
};
}  // namespace coreutils

#endif  // LIB_ARGUMENTPARSER_HPP_
