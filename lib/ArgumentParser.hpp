///
///  @file ArgumentParser.hpp
///  @brief public API for the coreutlspp-wide argument parser
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

#ifndef LIB_ARGUMENTPARSER_HPP_
#define LIB_ARGUMENTPARSER_HPP_

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <print>
#include <span>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "detail/ArgumentParser.hpp"

namespace coreutils {

template <class T, auto Converter, detail::ComptimeString... Names>
using MultiValueArgument =
    detail::Argument<T, detail::NArgs::Many, Converter, Names...>;

template <class T, auto Converter>
using PositionalArguments =
    detail::Argument<T, detail::NArgs::Many, Converter, "">;

template <detail::ComptimeString... Names>
using BooleanArgument = detail::Argument<void, detail::NArgs::None,
                                         [](std::string_view) {}, Names...>;

template <class T, auto Converter, detail::ComptimeString... Names>
using SingleValueArgument =
    detail::Argument<T, detail::NArgs::One, Converter, Names...>;

template <detail::ComptimeString HelpText, detail::ComptimeString... Names>
struct ArgumentInfo final {
    static inline constexpr std::array<std::string_view, sizeof...(Names)>
        names{Names.PrintableView()...};
    static inline constexpr std::string_view help_text{
        HelpText.PrintableView()};
};

template <class ConvertedType, auto Converter>
concept ValueConverter =
    std::regular_invocable<decltype(Converter), std::string_view> &&
    std::same_as<ConvertedType,
                 std::invoke_result_t<decltype(Converter), std::string_view>>;

template <class ConvertedType, auto Converter>
concept EmptyConverter = std::same_as<bool, ConvertedType> &&
                         std::same_as<decltype(Converter), nullptr_t>;

template <class ConvertedType>
concept NonVoid = !std::same_as<ConvertedType, void>;

template <class ConvertedType, auto Converter>
concept ArgumentConverter =
    NonVoid<ConvertedType> && (ValueConverter<ConvertedType, Converter> ||
                               EmptyConverter<ConvertedType, Converter>);

template <class ConvertedType, auto Converter>
    requires ArgumentConverter<ConvertedType, Converter>
struct ConversionInfo final {
    using type = ConvertedType;
    static inline constexpr decltype(Converter) Fn{Converter};
};

template <detail::ComptimeString Program, detail::ComptimeString Version,
          detail::ComptimeString Usage, detail::ComptimeString Summary>
struct ProgramInfo final {
    static inline constexpr detail::ComptimeString name{Program};
    static inline constexpr detail::ComptimeString version{Version};
    static inline constexpr detail::ComptimeString usage{Usage};
    static inline constexpr detail::ComptimeString summary{Summary};
};

template <class T, template <class, auto> class U>
struct is_conversion_info : std::false_type {};

template <class V, auto C, template <class, auto> class U>
struct is_conversion_info<U<V, C>, ConversionInfo> : std::true_type {};

template <class T>
decltype(auto) is_conversion_info_v{
    is_conversion_info<T, ConversionInfo>::value};

template <class T, template <detail::ComptimeString...> class U>
struct is_instance_of : std::false_type {};

template <detail::ComptimeString... Info>
struct is_instance_of<ProgramInfo<Info...>, ProgramInfo> : std::true_type {};

template <detail::ComptimeString... Info>
struct is_instance_of<ArgumentInfo<Info...>, ArgumentInfo> : std::true_type {};

template <class T>
concept IsProgramInfo = is_instance_of<T, ProgramInfo>::value;

template <class T>
concept IsArgumentInfo = is_instance_of<T, ArgumentInfo>::value;

template <class T>
concept IsFlagConverter =
    is_conversion_info_v<T> && std::same_as<typename T::type, bool> &&
    std::same_as<std::remove_cv_t<decltype(T::Fn)>, nullptr_t>;

template <class T>
concept IsValueConverter =
    is_conversion_info_v<T> && !std::same_as<typename T::type, void>;

template <IsArgumentInfo Info_t, IsFlagConverter Converter_t>
struct Flag {};

template <IsArgumentInfo Info_t, IsValueConverter Converter_t>
struct Option {};

template <class T>
concept Arg = requires(T arg) {
    std::same_as<std::string_view, decltype(T::help_view_)>;
    T::value;
};

template <IsProgramInfo Program, Arg... Args>
class ArgumentParser final {
 public:
    constexpr explicit ArgumentParser(int argc, const char** argv)
        : args_{argv + 1, argv + argc} {}

    constexpr auto ParseArgsOrExit() {
        for (std::string_view arg : args_) {
            if (arg == "--version") {
                PrintVersion();
            } else if (arg == "--help") {
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
        std::println("{} (coreutilspp) version {}\n\n{}",
                     Program::name.PrintableView(),
                     Program::version.PrintableView(), license_info_);
        std::exit(0);
    }

    [[noreturn]]
    constexpr void PrintHelp() const {
        std::println(Program::usage.PrintableView());
        std::println(Program::summary.PrintableView());
        std::println("\t{:<15}{:<}", "--help", "display this help and exit");
        std::println("\t{:<15}{:<}", "--version",
                     "output version information and exit");
        (std::println("\t{}", Args::help_view_), ...);
        std::exit(0);
    }

    template <class Out>
    constexpr Out& get() {
        return std::get<Out>(arg_values_);
    }

 private:
    // since argc and argv should be valid for the lifetime of the main
    // function, storing this should be safe, as the lifetime of this object
    // should logically always be less than the main function.
    std::span<const char*> args_{};
    std::tuple<Args...> arg_values_{};

    static constexpr std::string_view license_info_{
        "Copyright (C) 2025 Free Software Foundation, Inc.\nLicense GPLv3+: "
        "GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\nThis "
        "is free software: you are free to change and redistribute it.\nThere "
        "is NO WARRANTY, to the extent permitted by law."};
};
}  // namespace coreutils

#endif  // LIB_ARGUMENTPARSER_HPP_
