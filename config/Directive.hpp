#ifndef DIRECTIVE_HPP
#define DIRECTIVE_HPP

#include <memory>
#include <vector>
#include <string>
#include <iostream>

class BlockDirective;
class Directive {
private:
    std::string name;
    std::vector<std::string> parameters;
    BlockDirective *block;
public:
    Directive(const Directive& o);
    Directive(std::string, std::vector<std::string>, BlockDirective*);
    void    debug();
    ~Directive();
};

#endif
