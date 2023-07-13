#include "ConfigParser.hpp"
#include "Directive.hpp"
#include "BlockDirective.hpp"
#include "Token.hpp"
#include <memory>
#include <stdexcept>

ConfigParser::ConfigParser() {
    error = false;
}

ConfigParser::ConfigParser(std::vector<Token> tokens) {
    this->tokens = std::deque<Token>(tokens.begin(), tokens.end());
    error = false;
}

Token   ConfigParser::consume(token_type type) {
    Token token("ERROR", 0);
    if (tokens.size()) {
        token = tokens.front(); tokens.pop_front();
        if (token.getType() == type)
            return token;
    }
    std::string errMsg =  "Error: expected '" + Token::Literal(type) + "' token\n";
    if (tokens.size())
        errMsg += "Got: '" + token.debugLiteral() + "' at line:" + token.getLine() + "\n";
    throw std::runtime_error(errMsg);
}

bool    ConfigParser::check(token_type type) {
    return tokens.size() && tokens.front().getType() == type;
}

std::vector<std::string> ConfigParser::parseParameters() {
    std::vector<std::string> result;

    while (check(WORD)) {
        result.push_back(consume(WORD).getLiteral());
    }

    return result;
}

Directive   ConfigParser::parseDirective() {
    std::string name = consume(WORD).getLiteral();
    std::vector<std::string> parameters = parseParameters();
    std::auto_ptr<BlockDirective> block;

    if (check(LEFT_CURLY)) {
        consume(LEFT_CURLY);
        std::vector<Directive> directives;
        while (!check(RIGHT_CURLY)) {
            directives.push_back(parseDirective());
        }
        block.reset(new BlockDirective(directives));
        consume(RIGHT_CURLY);
    } else
        consume(SEMICOLON);

    return Directive(name, parameters, block.release());
}

void    ConfigParser::parse() {
    try {
        for (;;) {
            if (tokens.size() == 0)
                break;
            Directive directive = parseDirective();

            directive.debug();
            std::cout << std::endl;
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what();
    }
}
