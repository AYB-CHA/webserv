#pragma once
# include <string>
# include <cmath>
# include "ABase.hpp"

class Location : public ABase {
	private:
		std::vector<std::string> prefix;
		std::map<std::string, std::string> cgi_path;

	public:
        Location();
        Location(const Location& o);
        Location& operator=(const Location& o);
		~Location();



		const std::vector<std::string> &getPrefix(void) const;
		const std::map<std::string, std::string> &getCgiPath(void) const;

		void setPrefix(std::vector<std::string> prefix);
		void setCgiPath(std::string extension, std::string path);
};
