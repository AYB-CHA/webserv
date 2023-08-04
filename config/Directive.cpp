#include "Directive.hpp"
#include "BlockDirective.hpp"
#include <memory>
#include <vector>

Directive::Directive(Token token, std::vector<std::string> parameters, BlockDirective *block)
    : token(token), parameters(parameters), block(block) {}

Directive::Directive(const Directive& o) : token(o.token), name(o.name), parameters(o.parameters) {
    this->block = o.block ? new BlockDirective(*o.block) : NULL;
}

token_type  Directive::getType() {
    return this->token.getType();
}

std::string Directive::getName() {
    return this->name;
}

std::vector<std::string> Directive::getParameters() {
    return this->parameters;
}

BlockDirective* Directive::getBlock() {
    return this->block;
}

std::string Directive::getLine() {
    return this->token.getLine();
}

void    Directive::debug() {
    std::cout << "Directive name: {" << this->token.getLiteral() << "} ";
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
