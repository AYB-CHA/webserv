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
    static std::map<std::string, token_type> lockeywords;
    static std::map<std::string, token_type> srvkeywords;
    Config();
    void    init(std::string file_name);
};

#endif
