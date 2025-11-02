// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

#ifndef LIB_ARGUMENTPARSER_HPP_
#define LIB_ARGUMENTPARSER_HPP_

#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <print>
#include <span>
#include <string_view>
#include <tuple>

#include "detail/ArgumentParser.hpp"

namespace coreutils {

template <class T, auto Converter, detail::ComptimeString... Names>
using MultiValueArgument =
    detail::Argument<T, detail::NArgs::Many, Converter, Names...>;

template <class T, auto Converter>
using PositionalArguments =
    detail::Argument<T, detail::NArgs::Many, Converter, "">;

template <detail::ComptimeString... Names>
using BooleanArgument =
    detail::Argument<void, detail::NArgs::None, []() {}, Names...>;

template <class T, auto Converter, detail::ComptimeString... Names>
using SingleValueArgument =
    detail::Argument<T, detail::NArgs::One, Converter, Names...>;

template <class T>
concept Arg = requires(T arg) {
    std::same_as<std::string_view, decltype(T::help_view_)>;
    T::value;
};

template <detail::ComptimeString Program, detail::ComptimeString Version,
          detail::ComptimeString Usage, detail::ComptimeString Summary>
struct ProgramInfo final {
    static inline constexpr detail::ComptimeString name{Program};
    static inline constexpr detail::ComptimeString version{Version};
    static inline constexpr detail::ComptimeString usage{Usage};
    static inline constexpr detail::ComptimeString summary{Summary};
};

template <class T, template <detail::ComptimeString...> class U>
struct is_instance_of : std::false_type {};

template <detail::ComptimeString... Info>
struct is_instance_of<ProgramInfo<Info...>, ProgramInfo> : std::true_type {};

template <class T>
concept IsProgramInfo = is_instance_of<T, ProgramInfo>::value;

template <IsProgramInfo Program, Arg... Args>
class ArgumentParser final {
    static_assert(is_instance_of<Program, ProgramInfo>::value,
                  "Program template-parameter must be instantiated from the "
                  "ProgramInfo type");

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
        std::println("{} version {}\n\n{}", Program::name.PrintableView(),
                     Program::version.PrintableView(), license_info_);
        std::exit(0);
    }

    [[noreturn]]
    constexpr void PrintHelp() const {
        std::println(Program::usage.PrintableView());
        std::println(Program::summary.PrintableView());
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
