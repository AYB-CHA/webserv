#include <string>

#include "../config/Directive.hpp"
#include "../config/BlockDirective.hpp"

#include "../server/utils.hpp"
#include "../server/Server.hpp"
#include "../server/Location.hpp"


typedef enum flag {
    IN_SERV,
    IN_LOCATION
}   flag;

void visite_directive(std::vector<Directive>::iterator &it, Server& serv, flag f);
void match_location_directive (std::vector<Directive>::iterator &it1, Location &location) {
    std::vector<std::string> parameters = it1->getParameters();

    if (it1->getType() == ROOT) {
        // std::cout << "root : " << parameters[0] << std::endl;
        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
            location.setRoot(*it);
        }
    }
    else if (it1->getType() == ACCEPTED_METHODS) {
        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
            location.setAllowedMethods(*it);
        }
    }
    else if (it1->getType() == INDEX) {
        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
            location.setIndex(*it);
        }
    }
    else if (it1->getType() == ERROR_PAGE) {
        // std::cout << "error_page : " << parameters[0] << std::endl;
        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
            std::vector<std::string> s = utils::split(*it, ":");
            if (s.size() != 2)
                throw std::runtime_error("params erro");
            else {
                location.setErrorPage(utils::toInt(s[0]), s[1]);
            }
        }
    } // not shared
    else if (it1->getType() == AUTOINDEX) {
        if (parameters.size() != 1 && (parameters[0] != "true" || parameters[0] != "false"))
            throw std::runtime_error("params erro");;
        if (parameters[0] == "true")
            location.setAutoindex(true);
        else
            location.setAutoindex(false);
    }
    else if (it1->getType() == CGI) {
        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
            std::vector<std::string> s = utils::split(*it, ":");
            if (s.size() != 2)
                throw std::runtime_error("params erro");;
            location.setCgiPath(s[0], s[1]);
        }
    }
}
void match_serv_directive(std::vector<Directive>::iterator &it1, Server& serv) {
    std::vector<std::string> parameters = it1->getParameters();

    if (it1->getType() == LISTEN) {
        // std::cout << "listen : " << parameters[0] << std::endl;
        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
            std::vector<std::string> s = utils::split(*it, ":");
            if (s.size() == 1) {
                serv.setPort(utils::toInt(s[0]));
            } else if (s.size() == 2) {
                // port
                serv.setPort(utils::toInt(s[0]));
                //  TODO: ip add
                utils::strTrim(s[1]);
                serv.setHost(s[1]);
                // ...
            } else {
                throw std::runtime_error("params erro");;
            }
        }

    }
    else if (it1->getType() == ROOT) {
        // std::cout << "root : " << parameters[0] << std::endl;
        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
            serv.setRoot(*it);
        }
    }
    else if (it1->getType() == ACCEPTED_METHODS) {
        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
            serv.setAllowedMethods(*it);
        }
    }
    else if (it1->getType() == INDEX) {
        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
            serv.setIndex(*it);
        }
    }
    else if (it1->getType() == ERROR_PAGE) {
        // std::cout << "error_page : " << parameters[0] << std::endl;
        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
            std::vector<std::string> s = utils::split(*it, ":");
            if (s.size() != 2)
                throw std::runtime_error("params erro");
            else {
                serv.setErrorPage(utils::toInt(s[0]), s[1]);
            }
        }
    }
    else if (it1->getType() == SERVER_NAME) {
        for (std::vector<std::string>::iterator it = parameters.begin(); it != parameters.end(); ++it) {
            serv.setServerName(*it);
        }
    }
    else if (it1->getType() == LOCATION) {
        // recursive
        visite_directive(it1, serv, IN_LOCATION);
        // std::cout << "location : later" << std::endl;
    }

}

void visite_directive(std::vector<Directive>::iterator &it, Server& serv, flag f) {
    std::vector<Directive> directives = it->getBlock()->getDirectives();
    Location location;
    for (std::vector<Directive>::iterator it1 = directives.begin(); it1 != directives.end(); ++it1) {
        if (f == IN_SERV) {
            match_serv_directive(it1, serv);
        } else {
            match_location_directive(it1, location);
        }
    }
    if (f == IN_LOCATION)
        serv.setLocation(location);
}

// visit_dir(directives:)
//     loop: directive
//         switch(token)
//         case loc: loop
//                 Location loc
//                 loop: match_dir(loc)
//                 push_back(loc)
//         default: match_dir(serv)


void printservs(std::vector<Server> servers) {
    // server
    for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it) {
        std::cout << "================== not shared serv ===================" << std::endl;
        std::cout << it->getHost() << std::endl;
        std::cout << it->getPort() << std::endl;

        std::vector<std::string> server_name = it->getServerNames();
        for (std::vector<std::string>::iterator it1 = server_name.begin(); it1 != server_name.end(); ++it1) {
            std::cout << *it1 << "  ";
        }
        std::cout << std::endl;

        std::map<std::string, std::string> redir = it->getRedirect();
        for (std::map<std::string, std::string>::iterator it1 = redir.begin(); it1 != redir.end(); ++it1) {
            std::cout << it1->first << " -> " << it1->second << " | ";
        }
        std::cout << std::endl;

        std::cout << "================= shared serv =================" << std::endl;
        std::cout << it->getRoot() << std::endl;
        std::cout << it->getUploadPath() << std::endl;

        std::map<std::string, bool> method = it->getAllowedMethods();
        for (std::map<std::string, bool>::iterator it1 = method.begin(); it1 != method.end(); ++it1) {
            std::cout << it1->first << " -> " << it1->second << " | ";
        }
        std::cout << std::endl;

        std::vector<std::string> index = it->getIndex();
        for (std::vector<std::string>::iterator it1 = index.begin(); it1 != index.end(); ++it1) {
            std::cout << *it1 << "  ";
        }
        std::cout << std::endl;

        std::map<int, std::string> errorpage = it->getErrorPage();
        for (std::map<int, std::string>::iterator it1 = errorpage.begin(); it1 != errorpage.end(); ++it1) {
            std::cout << it1->first << " -> " << it1->second << " | ";
        }
        std::cout << std::endl;

        std::cout << it->getClientMaxBodySize() << std::endl;

        // location
        std::vector<Location> location = it->getLocation();
        for (std::vector<Location>::iterator it1 = location.begin(); it1 != location.end(); ++it1) {
            std::cout << "================== not shared loc===================" << std::endl;
            std::cout << it1->getPrefix() << std::endl;
            std::cout << it1->getAutoindex() << std::endl;

            std::map<std::string, std::string> cgi = it1->getCgiPath();
            for (std::map<std::string, std::string>::iterator it2 = redir.begin(); it2 != redir.end(); ++it2) {
                std::cout << it2->first << " -> " << it2->second << " | ";
            }
            std::cout << std::endl;

            std::cout   << "============= shared loc ============" << std::endl;
            std::cout << it1->getRoot() << std::endl;
            std::cout << it1->getUploadPath() << std::endl;

            std::map<std::string, bool> method = it1->getAllowedMethods();
            for (std::map<std::string, bool>::iterator it1 = method.begin(); it1 != method.end(); ++it1) {
                std::cout << it1->first << " -> " << it1->second << " | ";
            }
            std::cout << std::endl;

            std::vector<std::string> index = it1->getIndex();
            for (std::vector<std::string>::iterator it1 = index.begin(); it1 != index.end(); ++it1) {
                std::cout << *it1 << "  ";
            }
            std::cout << std::endl;

            std::map<int, std::string> errorpage = it1->getErrorPage();
            for (std::map<int, std::string>::iterator it1 = errorpage.begin(); it1 != errorpage.end(); ++it1) {
                std::cout << it1->first << " -> " << it1->second << " | ";
            }
            std::cout << std::endl;

            std::cout << it1->getClientMaxBodySize() << std::endl;
        }
        std::cout << std::endl;

    }

}



void validator(std::vector<Directive> _servers) {

    std::vector<Server> servers;
    for (std::vector<Directive>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
        Server serv;
        visite_directive(it, serv, IN_SERV);
        servers.push_back(serv);
        std::cout << "++++++++++++++++++++++++ server +++++++++++++++++++++++++++" << std::endl;
    }
    printservs(servers);
}

// bool listen(std::string listen) {

// }
