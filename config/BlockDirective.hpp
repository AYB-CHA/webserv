#ifndef BLOCKDIRECTIVE_HPP
#define BLOCKDIRECTIVE_HPP

#include "Directive.hpp"
#include <vector>

class BlockDirective {
private:
    static int depth;
    std::vector<Directive> directives;
public:
    BlockDirective();
    BlockDirective(const BlockDirective& o);
    BlockDirective(std::vector<Directive>);
    std::vector<Directive> getDirectives();

    void    debug();
};

#endif
