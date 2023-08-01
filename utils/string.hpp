#pragma once
#include <iostream>
#include <sstream>

namespace utils {
namespace string {
bool hasNoSpace(const std::string &string);
int toInt(const std::string &string);
std::string fromInt(int integer);
std::string toHex(int number);
} // namespace string
} // namespace utils
