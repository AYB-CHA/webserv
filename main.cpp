#include "main.hpp"

int main(int ac, char **av) {
  (void)ac;
  if (ac != 2) {
    std::cerr << "Usage: " << av[0] << " <config-file>" << std::endl;
    return 1;
  }
  Config config;
  config.init(av[1]);

  std::vector<Directive> servers_directives = config.getServers();

  std::vector<Server> servers;
  Server server;
  server.setHost("127.0.0.1");
  server.setPort(9000);

  servers.push_back(server);

  try {
    Core core(servers);
  } catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
