#include "BlockDirective.hpp"
#include <vector>

int BlockDirective::depth = 0;

BlockDirective::BlockDirective() {}

BlockDirective::BlockDirective(std::vector<Directive> dir)
        : directives(dir) {}

BlockDirective::BlockDirective(const BlockDirective& o) {
    directives = std::vector<Directive>(o.directives.begin(), o.directives.end());
}

void    BlockDirective::debug() {
    depth++;
    for (std::vector<Directive>::iterator it = directives.begin(); it != directives.end(); ++it) {
        for (int i = 0; i < depth; i++)
            std::cout << "   ";
        it->debug();
        std::cout << std::endl;
    }
    depth--;
}
