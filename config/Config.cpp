#include "Config.hpp"

Config::Config() {}

void    Config::init(std::string file_name) {
    lexer.scanFile(file_name);
    lexer.tokenize();

    parser = ConfigParser(lexer.getTokens());
    parser.parse();
}
