#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

typedef enum token_type {
    WORD,
    SEMICOLON,
    LEFT_CURLY,
    RIGHT_CURLY,
    SERVER,
    LOCATION,
    LISTEN,
    ROOT,
}   token_type;

class Token {
private:
    std::string literal;
    token_type type;
    int         line;
public:
    Token(std::string literal, int line);
    Token(token_type type, int line);
    std::string getLiteral();
    token_type getType();
    std::string getLine();
    std::string debugLiteral();
    static std::string Literal(token_type type);
    ~Token();
};

#endif
