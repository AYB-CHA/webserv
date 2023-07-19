#pragma once
# include <string>
# include <cmath>
# include "ABase.hpp"

class Location : public ABase {
	private:
		bool autoindex;
		std::vector<std::string> prefix;
		std::map<std::string, std::string> cgi_path;

	public:
        Location();
		~Location();

		const std::vector<std::string> &getPrefix(void) const;
		bool getAutoindex(void) const;
		const std::map<std::string, std::string> &getCgiPath(void) const;

		void setPrefix(std::vector<std::string> prefix);
		void setAutoindex(bool autoindex);
		void setCgiPath(std::string extension, std::string path);
};
