#include "utils.hpp"
#include <iostream>
#include <sstream>


int utils::toInt(std::string nb) {
    std::stringstream ss(nb);
    int num;
    ss >> num;

    if(!ss.eof() || ss.fail()) {
        return -1;
    }
    return num;
}

std::string utils::toString(long nb) {
    std::stringstream ss;
    std::string str;

    ss << nb;
    ss >> str;

    return str;
}

void utils::strTrim(std::string& str) {
    std::string result = str;
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    str = result;
}

std::vector<std::string> utils::split(std::string str, std::string delimiter) {

    std::vector<std::string> list;
    std::size_t start = 0;

    strTrim(str);

    if (str.size() == 0)
        return list;

    std::size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        std::string holder = str.substr(start, end - start);
        if (!holder.empty())
            list.push_back(holder);
        start = end + 1;
        end = str.find(delimiter, start);
    }
    if (!(str.substr(start, str.size()).empty()))
        list.push_back(str.substr(start, str.size()));

    return list;
}




// int main () {
// 	std::string s("/www/da/index.html/");
// 	std::vector<std::string> vec = utils::split(s, "/");
// 	for(std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); ++it) {
// 		std::cout << "item: " << *it << std::endl;
// 	}

// 	// std::cout << utils::toInt("347") << std::endl;

// }
