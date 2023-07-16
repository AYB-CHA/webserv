#include "main.hpp"
#include <cstdlib>

int main(int ac, char **av) {
  (void) ac;
    if (ac != 2) {
        std::cerr << "Usage: " << av[0] << " <config-file>" << std::endl;
        return 1;
    }
    Config config;
    config.init(av[1]);

	std::vector<Directive> servers = config.getServers();
  return 0;
}
