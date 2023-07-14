#pragma once
# include <string>
# include "ABase.hpp"

class Location : public ABase {
	private:
		std::string prefix;
		bool autoindex;
		std::map<std::string, std::string> cgi_path;

	public:

		~Location();

		const std::string &getPrefix(void) const;
		bool getAutoindex(void) const;
		const std::map<std::string, std::string> &getCgiPath(void) const;

		void setPrefix(std::string prefix);
		void setAutoindex(bool autoindex);
		void setCgiPath(std::string extension, std::string path);
};
