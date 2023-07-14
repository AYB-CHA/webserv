#include "Token.hpp"
#include <sstream>

Token::Token(std::string literal, int line) {
    this->literal = literal;
    this->type = WORD;
    this->line = line;
}

Token::Token(std::string literal, token_type type, int line) {
    this->literal = literal;
    this->type = type;
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
        case WORD: return "parameter";
        case LEFT_CURLY: return "{";
        case RIGHT_CURLY: return "}";
        case SEMICOLON: return ";";
        case SERVER: return "server";
        case LISTEN: return "listen";
        case LOCATION: return "location";
        case ROOT: return "root";
        case INDEX: return "index";
        case SERVER_NAME: return "server_name";
        case ACCEPTED_METHODS: return "accepted_methods";
        case ERROR_PAGE: return "error_page";
        case AUTOINDEX: return "autoindex";
        case CGI: return "cgi";
    }
    return "unknown token";
}

Token::~Token() {}
