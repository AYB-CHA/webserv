#include "Directive.hpp"
#include "BlockDirective.hpp"
#include <memory>
#include <vector>

Directive::Directive(std::string name, std::vector<std::string> parameters, BlockDirective *block)
    : name(name), parameters(parameters), block(block) {}

Directive::Directive(token_type token, std::vector<std::string> parameters, BlockDirective *block)
    : token(token), parameters(parameters), block(block) {}

Directive::Directive(const Directive& o) : token(o.token), name(o.name), parameters(o.parameters) {
    this->block = o.block ? new BlockDirective(*o.block) : NULL;
}

void    Directive::debug() {
    if (this->token == WORD)
        std::cout << "Directive name: {" << name << "} ";
    else
        std::cout << "Directive name: -{" << Token::Literal(this->token) << "} ";
    std::cout << "Parameters:";
    for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
        std::cout << "(" << *it << ")";
        if (it + 1 != parameters.end())
            std::cout << "-";
    }
    std::cout << std::endl;
    if (block)
        block->debug();
}

Directive::~Directive() {
    delete block;
}
