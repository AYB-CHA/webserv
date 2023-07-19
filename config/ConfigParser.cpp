#include "ConfigParser.hpp"
#include "Config.hpp"
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
    std::string errMsg =  "Expected '" + Token::Literal(type) + "' token\n";
    if (tokens.size())
        errMsg += "Got: '" + token.getLiteral() + "' at line: " + token.getLine() + "\n";
    throw std::runtime_error(errMsg);
}

bool    ConfigParser::check(token_type type) {
    return tokens.size() && tokens.front().getType() == type;
}

std::vector<std::string> ConfigParser::parseParameters() {
    std::vector<std::string> result;

    if (!check(WORD)) consume(WORD);
    while (check(WORD)) {
        result.push_back(consume(WORD).getLiteral());
    }

    return result;
}

token_type  ConfigParser::consumeSrvKeyword() {
    if (!tokens.size()) throw std::runtime_error("Expected directive.");

    Token token = tokens.front(); tokens.pop_front();

    if (Config::srvkeywords.find(token.getLiteral()) == Config::srvkeywords.end())
        throw std::runtime_error(std::string("Invalid directive at token: '") + token.getLiteral()
                                 + "' at line: " + token.getLine());
    return token.getType();
}

token_type  ConfigParser::consumeLocKeyword() {
    if (!tokens.size()) throw std::runtime_error("Expected directive.");

    Token token = tokens.front(); tokens.pop_front();

    if (Config::lockeywords.find(token.getLiteral()) == Config::lockeywords.end())
        throw std::runtime_error(std::string("Invalid directive at token: '") + token.getLiteral()
                                 + "' at line: " + token.getLine());
    return token.getType();
}

Directive   ConfigParser::parseSimpleDirective() {
    token_type token = consumeSrvKeyword();
    std::vector<std::string> parameters = parseParameters();

    consume(SEMICOLON);

    return Directive(token, parameters, NULL);
}

Directive   ConfigParser::parseLocDirective() {
    token_type token = consumeLocKeyword();
    std::vector<std::string> parameters = parseParameters();

    consume(SEMICOLON);

    return Directive(token, parameters, NULL);
}

Directive   ConfigParser::parseLocation() {
    token_type token = consumeSrvKeyword();
    std::vector<std::string> parameters = parseParameters();

    consume(LEFT_CURLY);
    std::vector<Directive> directives;
    while (!check(RIGHT_CURLY)) {
        directives.push_back(parseLocDirective());
    }
    consume(RIGHT_CURLY);

    return Directive(token, parameters, new BlockDirective(directives));
}

Directive  ConfigParser::parseServer() {
    token_type token = consume(SERVER).getType();
    std::vector<Directive> directives;

    consume(LEFT_CURLY);
    while (!check(RIGHT_CURLY)) {
        if (check(LOCATION))
            directives.push_back(parseLocation());
        else
            directives.push_back(parseSimpleDirective());
    }
    consume(RIGHT_CURLY);

    return Directive(token, std::vector<std::string>(), new BlockDirective(directives));
}

void    ConfigParser::parse() {
    while (tokens.size() > 0) {
        Directive directive = parseServer();

        servers.push_back(directive);
    }
}

std::vector<Directive> ConfigParser::getServers() {
    return this->servers;
}
