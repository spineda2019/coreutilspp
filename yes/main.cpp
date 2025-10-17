#include <print>
#include <span>
int main(int argc, const char **argv) {
  std::span<const char *> args{argv, argv + argc};

  for (const char *str : args) {
    std::println("Args: {}", str);
  }
  return 0;
}
