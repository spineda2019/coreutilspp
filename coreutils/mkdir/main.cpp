// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

#include <filesystem>

#include "lib/ArgumentParser.hpp"

int main(int argc, const char** argv) {
    using Verbose = coreutils::Argument<"-V", void, coreutils::NArgs::None>;
    using Dirs =
        coreutils::Argument<"", std::filesystem::path, coreutils::NArgs::Many>;

    coreutils::ArgumentParser<"mkdir", "0.0.1", Verbose, Dirs> parser{argc,
                                                                      argv};
    parser.ParseArgsOrExit();
}
