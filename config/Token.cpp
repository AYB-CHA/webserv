#include "Token.hpp"
#include <sstream>

Token::Token(std::string literal, int line) {
    this->literal = literal;
    this->type = WORD;
    this->line = line;
}

Token::Token(token_type type, int line) {
    this->type = type;
    this->line = line;
}

token_type Token::getType() {
    return this->type;
}

std::string Token::getLiteral() {
    return this->literal;
}

std::string Token::getLine() {
    std::stringstream ss;
    std::string line;
    ss << this->line;
    ss >> line;
    return line;
}

std::string Token::Literal(token_type type) {
    switch (type) {
        case WORD: return "DIR NAME";
        case LEFT_CURLY: return "{";
        case RIGHT_CURLY: return "}";
        case SEMICOLON: return ";";
        case SERVER: return "server";
        default: return "ERROR";
    }
}

std::string Token::debugLiteral() {
    switch (type) {
        case WORD: return literal;
        case LEFT_CURLY: return "{";
        case RIGHT_CURLY: return "}";
        case SEMICOLON: return ";";
        case SERVER: return "server";
        default: return "ERROR";
    }
}

Token::~Token() {}
