#pragma once
#include <iostream>
#include <sstream>

namespace utils {
class string {
public:
  static bool hasNoSpace(const std::string &string);
  static int toInt(const std::string &string);
  static std::string fromInt(int integer);
};
} // namespace utils
