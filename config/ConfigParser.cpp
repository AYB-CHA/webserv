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

token_type  ConfigParser::consumeKeyword() {
    if (!tokens.size()) throw std::runtime_error("Expected directive.");

    Token token = tokens.front(); tokens.pop_front();

    switch (token.getType()) {
        case LISTEN:
        case ROOT:
        case SERVER:
        case LOCATION: return token.getType();
        default: throw std::runtime_error("Invalid directive.");
    }
}

Directive   ConfigParser::parseSimpleDirective() {
    token_type token = consumeKeyword();
    std::vector<std::string> parameters = parseParameters();

    consume(SEMICOLON);

    return Directive(token, parameters, NULL);
}

Directive   ConfigParser::parseDirective() {
    token_type token = consumeKeyword();
    std::vector<std::string> parameters = parseParameters();
    std::auto_ptr<BlockDirective> block;

    if (check(LEFT_CURLY)) {
        consume(LEFT_CURLY);
        std::vector<Directive> directives;
        while (!check(RIGHT_CURLY)) {
            directives.push_back(parseSimpleDirective());
        }
        block.reset(new BlockDirective(directives));
        consume(RIGHT_CURLY);
    } else
        consume(SEMICOLON);

    return Directive(token, parameters, block.release());
}

Directive  ConfigParser::parseServer() {
    token_type token = consumeKeyword();
    std::vector<Directive> directives;
    std::auto_ptr<BlockDirective> block;

    if (token != SERVER)
        throw std::runtime_error("Invalid directive.");
    consume(LEFT_CURLY);
    while (!check(RIGHT_CURLY)) {
        if (check(LOCATION))
            directives.push_back(parseDirective());
        else
            directives.push_back(parseSimpleDirective());
    }
    block.reset(new BlockDirective(directives));
    consume(RIGHT_CURLY);

    return Directive(token, std::vector<std::string>(), block.release());
}

void    ConfigParser::parse() {
    try {
        while (tokens.size() > 0) {
            Directive directive = parseServer();

            directive.debug();
            std::cout << std::endl;
            servers.push_back(directive);
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

std::vector<Directive> ConfigParser::getServers() {
    return this->servers;
}
