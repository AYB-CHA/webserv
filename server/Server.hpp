#pragma once
# include <string>
# include <vector>
# include "Location.hpp"

class Server : public ABase {
	private:
		std::string host;
		int port;
		std::string server_name;
		std::string redirect;

		std::vector<Location> location;

		//-------------------------------------------------

		// map<int, Clinet> clients;
		// int socket_fd;

	public:

		~Server();

		const std::string &getHost(void) const;
		int getPort(void) const;
		const std::string &getServerName(void) const;
		const std::string &getRedirect(void) const;
		const std::vector<Location> &getLocation(void) const;

		void setHost(std::string host);
		void setPort(int port);
		void setServerName(std::string root);
		void setRedirect(std::string redirect);
		void setLocation(Location loc);
};

