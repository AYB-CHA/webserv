#pragma once
# include <string>
# include <vector>

namespace utils {

	int toInt(std::string nb);
	void strTrim(std::string& str);
	std::vector<std::string> split(std::string str, std::string delimiter);

};
