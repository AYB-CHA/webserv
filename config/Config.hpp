#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ConfigLexer.hpp"
#include "ConfigParser.hpp"
#include "Token.hpp"
#include <map>
class Config {
private:
    ConfigLexer lexer;
    ConfigParser parser;
public:
    static std::map<std::string, token_type> keywords;
    Config();
    void    init(std::string file_name);
};

#endif
