#pragma once
# include <string>
# include <vector>

namespace utils {

	int toInt(std::string nb);
    std::string toString(long nb);
	void strTrim(std::string& str);
    void strTrimV2(std::string& str, const std::string& charSet);
	std::vector<std::string> split(std::string str, std::string delimiter);

};
