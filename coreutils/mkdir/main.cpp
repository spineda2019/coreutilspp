// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@gmail.com)

#include "lib/ArgumentParser.hpp"

int main(int argc, const char** argv) {
    coreutils::ArgumentParser<"mkdir"> parser{argc, argv};
    (void)parser;
}
