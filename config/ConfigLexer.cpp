#include "ConfigLexer.hpp"
#include "Config.hpp"
#include "Token.hpp"
#include <fstream>
#include <stdexcept>

ConfigLexer::ConfigLexer() : line(1) {
    // SERVER KEYWORDS
    Config::srvkeywords["listen"] = LISTEN;
    Config::srvkeywords["index"] = INDEX;
    Config::srvkeywords["autoindex"] = AUTOINDEX;
    Config::srvkeywords["server_name"] = SERVER_NAME;
    Config::srvkeywords["accepted_methods"] = ACCEPTED_METHODS;
    Config::srvkeywords["root"] = ROOT;
    Config::srvkeywords["error_page"] = ERROR_PAGE;
    Config::srvkeywords["location"] = LOCATION;
    Config::srvkeywords["upload_path"] = UPLOAD_PATH;
    Config::srvkeywords["client_max_body_size"] = CLIENT_MAX_BODY_SIZE;

    // LOCATION KEYWORDS
    Config::lockeywords["root"] = ROOT;
    Config::lockeywords["index"] = INDEX;
    Config::lockeywords["autoindex"] = AUTOINDEX;
    Config::lockeywords["accepted_methods"] = ACCEPTED_METHODS;
    Config::lockeywords["cgi"] = CGI;
    Config::lockeywords["error_page"] = ERROR_PAGE;
    Config::lockeywords["upload_path"] = UPLOAD_PATH;
    Config::lockeywords["client_max_body_size"] = CLIENT_MAX_BODY_SIZE;
    Config::lockeywords["redirect"] = REDIRECT;
}

void    ConfigLexer::scanFile(std::string file_name) {
    std::fstream file(file_name.c_str());

    if (!file) {
        std::string errMsg = "Couldn't open file '" + file_name + "'";
        throw std::runtime_error(errMsg);
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
    if (value.size()) {
        if (value == "server") tokens.push_back(Token(SERVER, line));
        else if (Config::srvkeywords.find(value) != Config::srvkeywords.end())
            tokens.push_back(Token(value, Config::srvkeywords[value], line));
        else if (Config::lockeywords.find(value) != Config::lockeywords.end())
            tokens.push_back(Token(value, Config::lockeywords[value], line));
        else tokens.push_back(Token(value, line));
    }
    value = "";
}

void    ConfigLexer::tokenize() {
    for (int i = 0; contents[i]; i++) {
        switch (contents[i]) {
            case '{': generateToken();
                tokens.push_back(Token("{", LEFT_CURLY, line));
                break;

            case '}': generateToken();
                tokens.push_back(Token("}", RIGHT_CURLY, line));
                break;

            case ';': generateToken();
                tokens.push_back(Token(";", SEMICOLON, line));
                break;

            case '\n': line++;
            case '\t':
            case ' ': generateToken();
                break;

            case '#': generateToken();
                while (contents[i] && contents[i] != '\n')
                    i++;
                line++;
                break;
            default: value += contents[i];
        }
    }
}

std::vector<Token> ConfigLexer::getTokens() {
    return this->tokens;
}
