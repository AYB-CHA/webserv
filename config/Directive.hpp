#ifndef DIRECTIVE_HPP
#define DIRECTIVE_HPP

#include "Token.hpp"
#include <memory>
#include <vector>
#include <string>
#include <iostream>

class BlockDirective;
class Directive {
private:
    Token token;
    std::string name;
    std::vector<std::string> parameters;
    BlockDirective *block;
public:
    Directive(const Directive& o);
    Directive(Token, std::vector<std::string>, BlockDirective*);

    token_type getType();
    std::string getName();
    std::vector<std::string> getParameters();
    BlockDirective  *getBlock();
    std::string     getLine();

    void    debug();
    ~Directive();
};

#endif
