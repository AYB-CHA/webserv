#include "main.hpp"
#include "selector/Multiplexer.hpp"

#include <cstdlib>
#include <stdexcept>

int main(int ac, char **av) {
    if (ac != 2) {
        std::cerr << "Usage: " << av[0] << " [config-file]" << std::endl;
        return 1;
    }
    Config config;
    try {
        config.init(av[1]);
    } catch (std::runtime_error& e) {
        std::cerr << "Parse error:" << std::endl;
        std::cerr << e.what() << std::endl;
    }

    std::vector<Directive> server_directives = config.getServers();

    std::vector<Server> servers;

    try {
        servers = validator(server_directives);
        Core core(servers);
        Multiplexer multiplexer(servers);
        multiplexer.run();
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
