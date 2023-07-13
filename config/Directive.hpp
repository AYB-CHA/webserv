#ifndef DIRECTIVE_HPP
#define DIRECTIVE_HPP

#include <memory>
#include <vector>
#include <string>
#include <iostream>

// There's a high chance you might need to refactor this
// by using polymorphism instead
class BlockDirective;
class Directive {
private:
    std::string name;
    std::vector<std::string> parameters;
    BlockDirective *block;

    void    cleanBlock(BlockDirective *);
public:
    Directive(const Directive& o);
    Directive(std::string, std::vector<std::string>, BlockDirective*);
    void    debug();
    void    cleanup();
    ~Directive();
};

#endif
