#include "main.hpp"

int main(int ac, char **av) {
  (void) ac;
    if (ac != 2) {
        std::cerr << "Usage: " << av[0] << " <config-file>" << std::endl;
        return 1;
    }
    Config config;
    config.init(av[1]);
  return 0;
}
