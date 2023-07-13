#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ConfigLexer.hpp"
#include "ConfigParser.hpp"
class Config {
private:
    ConfigLexer lexer;
    ConfigParser parser;
public:
    Config();
    void    init(std::string file_name);
};

#endif
