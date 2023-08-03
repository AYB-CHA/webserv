#include "string.hpp"

bool utils::string::hasNoSpace(const std::string &string) {
    for (size_t i = 0; i < string.length(); i++) {
        if (string[i] == ' ')
            return false;
    }
    return true;
}

std::string utils::string::toHex(int number) {
    std::stringstream ss;
    std::string result;

    ss << std::hex << number;
    ss >> result;
    return result;
}

int utils::string::toInt(const std::string &string) {
    int integer;

    std::stringstream ss(string);
    ss >> integer;
    return integer;
}

std::string utils::string::fromInt(int integer) {
    std::stringstream ss;
    ss << integer;
    return ss.str();
}

std::string utils::string::toUpperCase(const std::string &string) {
    std::string ret_string;
    for (size_t i = 0; i < string.length(); i++)
        ret_string += std::toupper(string[i]);
    return ret_string;
}