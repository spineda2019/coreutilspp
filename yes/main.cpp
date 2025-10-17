// Copyright 2025 Sebastian Pineda (spineda.wpi.alum@wpi.edu)

#include <print>

int main(int argc, const char **argv) {
  if (argc > 2) {
    std::println("Usage: yes [string]");
    return 1;
  }

  const char *to_print{"y"};

  switch (argc) {
  case 1:
    break;
  case 2:
    to_print = argv[1];
    break;
  default:
    std::println("Usage: yes [string]");
    return 1;
  }

  while (true) {
    std::println("{}", to_print);
  }

  return 0;
}
