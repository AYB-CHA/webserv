#include "Config.hpp"

Config::Config() {}

std::map<std::string, token_type> Config::keywords;

void    Config::init(std::string file_name) {
    lexer.scanFile(file_name);
    lexer.tokenize();

    parser = ConfigParser(lexer.getTokens());
    parser.parse();
}
