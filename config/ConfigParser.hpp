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
    std::vector<Directive> servers;
    bool              error;

    // PARSERS
    Directive           parseServer();
    Directive           parseDirective();
    Directive           parseSimpleDirective();
    std::vector<std::string> parseParameters();

    // UTILS
    token_type      consumeKeyword();
    Token           consume(token_type);
    bool            check(token_type type);
public:
    ConfigParser();
    ConfigParser(std::vector<Token> tokens);
    void    parse();
    std::vector<Directive> getServers();
};

#endif
