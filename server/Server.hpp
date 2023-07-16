#pragma once
# include <string>
# include <vector>
# include "Location.hpp"

class Server : public ABase {
	private:
		std::string host;
		int port;
		std::vector<std::string> server_names;
		std::map<std::string, std::string> redirect;

		std::vector<Location> location;

		//-------------------------------------------------

		// map<int, Clinet> clients;
		// int socket_fd;

	public:

		~Server();

		const std::string &getHost(void) const;
		int getPort(void) const;
		const std::vector<std::string> &getServerNames(void) const;
		const std::map<std::string,std::string> &getRedirect(void) const;
		const std::vector<Location> &getLocation(void) const;

		void setHost(std::string host);
		void setPort(int port);
		void setServerName(std::string server_name);
		void setRedirect(std::string form, std::string to);
		void setLocation(Location loc);
};

