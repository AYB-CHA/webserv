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
    token_type token;
    std::string name;
    std::vector<std::string> parameters;
    BlockDirective *block;
public:
    Directive(const Directive& o);
    Directive(std::string, std::vector<std::string>, BlockDirective*);
    Directive(token_type, std::vector<std::string>, BlockDirective*);

    token_type getType();
    std::string getName();
    std::vector<std::string> getParameters();
    BlockDirective  *getBlock();

    void    debug();
    ~Directive();
};

#endif
