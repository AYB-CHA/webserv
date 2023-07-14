#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

typedef enum token_type {
    WORD,
    SEMICOLON,
    LEFT_CURLY,
    RIGHT_CURLY,
    SERVER,
    LISTEN,
    INDEX,
    SERVER_NAME,
    ACCEPTED_METHODS,
    ROOT,
    ERROR_PAGE,
    LOCATION,
    AUTOINDEX,
    CGI,
}   token_type;

class Token {
private:
    std::string literal;
    token_type type;
    int         line;
public:
    Token(std::string literal, token_type type, int line);
    Token(std::string literal, int line);
    Token(token_type type, int line);
    std::string getLiteral();
    token_type getType();
    std::string getLine();
    static std::string Literal(token_type type);
    ~Token();
};

#endif
