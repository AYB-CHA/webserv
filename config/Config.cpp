#include "Config.hpp"

Config::Config() {}

std::map<std::string, token_type> Config::lockeywords;
std::map<std::string, token_type> Config::srvkeywords;

void    Config::init(std::string file_name) {
    lexer.scanFile(file_name);
    lexer.tokenize();

    parser = ConfigParser(lexer.getTokens());
    parser.parse();
}

std::vector<Directive>  Config::getServers() {
    return parser.getServers();
}
