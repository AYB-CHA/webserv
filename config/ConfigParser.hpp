#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <vector>
#include <deque>
#include <iostream>
#include "Directive.hpp"
#include "Token.hpp"

class ConfigParser {
private:
    std::deque<Token> tokens;
    bool              error;

    // PARSERS
    Directive           parseServer();
    Directive           parseDirective();
    std::vector<std::string> parseParameters();

    // UTILS
    Token           consume(token_type);
    bool            check(token_type type);
public:
    ConfigParser();
    ConfigParser(std::vector<Token> tokens);
    void    parse();
};

#endif
