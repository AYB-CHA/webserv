#include "ConfigLexer.hpp"
#include "Token.hpp"
#include <fstream>

ConfigLexer::ConfigLexer() : line(1) {}

void    ConfigLexer::scanFile(std::string file_name) {
    std::fstream file(file_name.c_str());

    if (!file) {
        std::cout << "Error: couldn't open file." << std::endl;
        return;
    }

    while (true) {
        std::string s;
        std::getline(file, s);
        if (file.eof())
            break;
        contents += s + "\n";
    }
}

void    ConfigLexer::generateToken() {
    if (value.size())
        tokens.push_back(Token(value, line));
    value = "";
}

void    ConfigLexer::tokenize() {
    for (int i = 0; contents[i]; i++) {
        switch (contents[i]) {
            case '{': generateToken();
                tokens.push_back(Token(LEFT_CURLY, line));
                break;

            case '}': generateToken();
                tokens.push_back(Token(RIGHT_CURLY, line));
                break;

            case ';': generateToken();
                tokens.push_back(Token(SEMICOLON, line));
                break;

            case '\n': line++;
            case ' ': generateToken();
                break;

            case '#': generateToken();
                while (contents[i] && contents[i] != '\n')
                    i++;
                break;
            default: value += contents[i];
        }
    }
}

std::vector<Token> ConfigLexer::getTokens() {
    return this->tokens;
}
