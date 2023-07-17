#include "string.hpp"

bool utils::string::hasNoSpace(const std::string &string) {
    for (size_t i = 0; i < string.length(); i++) {
        if (string[i] == ' ')
            return false;
    }
    return true;
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
