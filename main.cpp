#include "main.hpp"
#include "selector/Multiplexer.hpp"

#include <cstdlib>
#include <stdexcept>

int main(int ac, char **av) {
    if (ac != 2) {
        std::cerr << "Usage: " << av[0] << " [config-file]" << std::endl;
        return 1;
    }
    std::vector<Directive> server_directives;
    std::vector<Server> servers;
    try {
        Config config;
        config.init(av[1]);
        server_directives = config.getServers();
    } catch (std::runtime_error& e) {
        std::cerr << "Parse error:" << std::endl;
        std::cerr << e.what() << std::endl;
    }



    try {
        servers = validator(server_directives);
        server_directives.clear();
        Core core(servers);
        Multiplexer multiplexer(servers);
        multiplexer.run();
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
