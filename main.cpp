#include "main.hpp"
#include "selector/Multiplexer.hpp"

#include <cstdlib>
#include <stdexcept>

void mem() { system("leaks webserv"); }

int main(int ac, char **av) {
    atexit(mem);
    if (ac > 2) {
        std::cerr << "Usage: " << av[0] << " [config-file]" << std::endl;
        return 1;
    }
    std::string configFile = "config.conf";
    if (ac == 2) {
        configFile = av[1];
    }
    std::vector<Directive> server_directives;
    std::vector<Server> servers;
    try {
        Config config;
        config.init(configFile);
        server_directives = config.getServers();
    } catch (std::runtime_error &e) {
        std::cerr << "Parse error:" << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
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
