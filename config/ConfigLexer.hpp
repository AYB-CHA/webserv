#ifndef CONFIGLEXER_HPP
#define CONFIGLEXER_HPP

#include <vector>
#include "Token.hpp"
#include <iostream>

class ConfigLexer {
private:
    std::vector<Token> tokens;
    std::string contents;
    std::string value;
    int         line;
    void        generateToken();
public:
    ConfigLexer();
    void    tokenize();
    void    scanFile(std::string file_name);
    std::vector<Token> getTokens();
};

#endif
