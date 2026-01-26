#include <ArgumentParser.hpp>
#include <array>
#include <functional>

namespace {
// -----------------------------------------------------------------------------
// Test 1: Boolean Flags (Presence/Absence)
// Description: Verifies that a flag defined via TMP is correctly detected
// when present and false when absent.
// -----------------------------------------------------------------------------
bool test_boolean_flag() {
    using namespace coreutils;
    // Definition: A parser with a single boolean flag "verbose"
    using Info = ProgramInfo<"test", "0.0.1", "test", "test">;
    using Verbose = BooleanArgument<"-v", "--verbose">;

    // Case A: Flag is present
    {
        constexpr int argc = 2;
        std::array<const char*, argc> argv{"Program", "--verbose"};

        ArgumentParser<Info, Verbose> parser{argc, argv.data()};
        try {
            parser.ParseArgsOrExit();
        } catch (...) {
            return false;
        }

        return parser.get<Verbose>().value;
    }

    // Case B: Flag is absent
    {
        constexpr int argc = 1;
        std::array<const char*, argc> argv{"Program"};

        ArgumentParser<Info, Verbose> parser{argc, argv.data()};
        try {
            parser.ParseArgsOrExit();
        } catch (...) {
            return false;
        }

        return !parser.get<Verbose>().value;
    }
}

/*
// -----------------------------------------------------------------------------
// Test 2: Typed Options (String & Integer)
// Description: Tests parsing value pairs and compile-time type conversion.
// -----------------------------------------------------------------------------
bool test_typed_options() {
    // Definition: Parser expecting a string "host" and an int "port"
    using MyParser =
        ArgParser<Option<"host", std::string>, Option<"port", int>>;

    MockArgs args({"./prog", "--host", "localhost", "--port", "8080"});
    auto result = MyParser::parse(args.argc, args.argv.data());

    if (!result.success) {
        std::cerr << "Parse failed unexpectedly." << std::endl;
        return false;
    }

    // Check String
    if (result.get<"host">() != "localhost") return false;

    // Check Integer conversion
    if (result.get<"port">() != 8080) return false;

    return true;
}

// -----------------------------------------------------------------------------
// Test 3: Short Flag Aliases
// Description: Verifies that short codes ('v') map to the long name
// ("verbose").
// -----------------------------------------------------------------------------
bool test_short_flags() {
    // Definition: "verbose" has short code 'v', "level" has short code 'l'
    using MyParser = ArgParser<Flag<"verbose", 'v'>, Option<"level", int, 'l'>>;

    MockArgs args({"./prog", "-v", "-l", "3"});
    auto result = MyParser::parse(args.argc, args.argv.data());

    if (!result.success) return false;
    if (result.get<"verbose">() != true) return false;
    if (result.get<"level">() != 3) return false;

    return true;
}

// -----------------------------------------------------------------------------
// Test 4: Mixed Ordering
// Description: Ensures argument order does not affect parsing logic.
// -----------------------------------------------------------------------------
bool test_mixed_ordering() {
    using MyParser = ArgParser<Flag<"force">, Option<"input", std::string>>;

    // Pass flag *after* the value option
    MockArgs args({"./prog", "--input", "data.txt", "--force"});
    auto result = MyParser::parse(args.argc, args.argv.data());

    if (!result.success) return false;
    if (result.get<"force">() != true) return false;
    if (result.get<"input">() != "data.txt") return false;

    return true;
}

// -----------------------------------------------------------------------------
// Test 5: Validation Failure (Missing Required)
// Description: Tests that the parser correctly returns failure/false when
// a required argument is missing.
// -----------------------------------------------------------------------------
bool test_missing_required() {
    // Definition: "config" is required
    using MyParser = ArgParser<Option<"config", std::string, Required>>;

    MockArgs args({"./prog"});  // No arguments provided
    auto result = MyParser::parse(args.argc, args.argv.data());

    // Expect failure
    if (result.success == true) return false;

    return true;
}

// -----------------------------------------------------------------------------
// Test 6: Negative Numbers vs Flags
// Description: Edge case where a negative number value looks like a flag.
// -----------------------------------------------------------------------------
bool test_negative_numbers() {
    using MyParser = ArgParser<Option<"offset", int>>;

    // "-5" should be parsed as value -5, not a flag named '5'
    MockArgs args({"./prog", "--offset", "-5"});
    auto result = MyParser::parse(args.argc, args.argv.data());

    if (!result.success) return false;
    if (result.get<"offset">() != -5) return false;

    return true;
}
*/

std::array<std::function<bool()>, 1> tests{test_boolean_flag};
}  // namespace

extern "C" {
bool test_argparser() {
    bool result{true};
    for (const auto& test : tests) {
        result = result && test();
    }

    return result;
}
}
