#include <string>

#include "../config/Directive.hpp"
#include "../config/BlockDirective.hpp"

#include "../server/utils.hpp"
#include "../server/Server.hpp"
#include "../server/Location.hpp"

void root_check(std::vector<std::string> params, ABase& base) {
    if (params.size() != 1)
        throw std::runtime_error("params error");
    base.setRoot(params.front());
}

void upload_path_check(std::vector<std::string> params, ABase& base) {
    if (params.size() != 1)
        throw std::runtime_error("params error");
    base.setUploadPath(params.front());
}

void accepted_methods_check(std::vector<std::string> params, ABase& base) {
    for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); ++it) {
        if (*it != "GET" && *it != "POST" && *it != "DELETE")
            throw std::runtime_error("params error");
        base.setAllowedMethods(*it);
    }
}

void index_check(std::vector<std::string> params, ABase& base) {
    for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); ++it) {
        base.setIndex(*it);
    }
}

void error_page_check(std::vector<std::string> params, ABase& base) {
    for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); ++it) {
        std::vector<std::string> s = utils::split(*it, ":");
        if (s.size() != 2 || utils::toInt(s[0]) < 0)
            throw std::runtime_error("params erro");
        else
            base.setErrorPage(utils::toInt(s[0]), s[1]);
    }
}

bool valid_size(std::string str) {
    if (str.back() != 'M' && str.back() != 'K' &&str.back() != 'G' && str.back() != 'B')
        return false;
    for (size_t i = 0; i < str.size() - 1; ++i) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}
void client_max_body_size_check(std::vector<std::string> params, ABase& base) {
    if (params.size() != 1 || !valid_size(params.front()))
        throw std::runtime_error("params error");
    base.setClientMaxBodySize(params.front());
}

bool valid_ip_addr(std::string str) {
    std::vector<std::string> s = utils::split(str, ".");
    if(s.size() != 4)
        return false;
    if(utils::toInt(s[0]) > 255 || utils::toInt(s[0]) < 0
        || utils::toInt(s[1]) > 255 || utils::toInt(s[1]) < 0
        || utils::toInt(s[2]) > 255 || utils::toInt(s[2]) < 0
        || utils::toInt(s[3]) > 255 || utils::toInt(s[3]) < 0)
        return false;
    return true;
}
void listen_check(std::vector<std::string> params, ABase& base) {
    for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); ++it) {
        std::vector<std::string> s = utils::split(*it, ":");
        if (s.size() == 1) {
            (dynamic_cast<Server&>(base)).setPort(utils::toInt(s.front()));
        } else if (s.size() == 2 && valid_ip_addr(s.front())) {
            (dynamic_cast<Server&>(base)).setPort(utils::toInt(s[1])); // port
            (dynamic_cast<Server&>(base)).setHost(s.front()); // ip addr
        } else {
            throw std::runtime_error("params erro");
        }
    }
}

void server_name_check(std::vector<std::string> params, ABase& base) {
    for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); ++it) {
        (dynamic_cast<Server&>(base)).setServerName(*it);
    }
}

void redirect_check(std::vector<std::string> params, ABase& base) {
    for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); ++it) {
        std::vector<std::string> s = utils::split(*it, ":");
        if (s.size() != 2)
            throw std::runtime_error("params erro");
        (dynamic_cast<Server&>(base)).setRedirect(s[0], s[1]);
    }
}

void autoindex_check(std::vector<std::string> params, ABase& base) {
    if (params.size() != 1 && (params[0] != "true" || params[0] != "false"))
        throw std::runtime_error("params erro");
    if (params[0] == "true")
        (dynamic_cast<Location&>(base)).setAutoindex(true);
    else
        (dynamic_cast<Location&>(base)).setAutoindex(false);
}

void cgi_check(std::vector<std::string> params, ABase& base) {
    for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); ++it) {
        std::vector<std::string> s = utils::split(*it, ":");
        if (s.size() != 2)
            throw std::runtime_error("params erro");
        (dynamic_cast<Location&>(base)).setCgiPath(s[0], s[1]);
    }
}


void match_dir(std::vector<Directive>::iterator &it1, ABase& base) {
    std::vector<std::string> params = it1->getParameters();
    switch (it1->getType()) {
        case ROOT: root_check(params, base); break;
        case UPLOAD_PATH: upload_path_check(params, base); break;
        case ACCEPTED_METHODS: accepted_methods_check(params, base); break;
        case INDEX: index_check(params, base); break;
        case ERROR_PAGE: error_page_check(params, base); break;
        case CLIENT_MAX_BODY_SIZE: client_max_body_size_check(params, base); break;
        case LISTEN: listen_check(params, base); break;
        case SERVER_NAME: server_name_check(params, base); break;
        case REDIRECT: redirect_check(params, base); break;
        case AUTOINDEX: autoindex_check(params, base); break;
        case CGI: cgi_check(params, base); break;
        default: break;
    }
}

void match_loc(Server& serv, std::vector<Directive>::iterator &it1) {
    Location loc;

    loc.setPrefix(it1->getParameters());
    std::vector<Directive> directives1 = it1->getBlock()->getDirectives();
    for (std::vector<Directive>::iterator it2 = directives1.begin(); it2 != directives1.end(); ++it2)
        match_dir(it2, loc);
    serv.setLocation(loc);
}

void visite_directive(std::vector<Directive>::iterator &it, Server& serv) {
    std::vector<Directive> directives = it->getBlock()->getDirectives();
    for (std::vector<Directive>::iterator it1 = directives.begin(); it1 != directives.end(); ++it1) {
        switch (it1->getType()) {
            case LOCATION: match_loc(serv, it1); break;
            default: match_dir(it1, serv); break;
        }
    }
}


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

            std::vector<std::string> prefix = it1->getPrefix();
            for (std::vector<std::string>::iterator it2 = prefix.begin(); it2 != prefix.end(); ++it2) {
                std::cout << *it2 << "  ";
            }
            std::cout << std::endl;

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
            for (std::map<std::string, bool>::iterator it2 = method.begin(); it2 != method.end(); ++it2) {
                std::cout << it2->first << " -> " << it2->second << " | ";
            }
            std::cout << std::endl;

            std::vector<std::string> index = it1->getIndex();
            for (std::vector<std::string>::iterator it2 = index.begin(); it2!= index.end(); ++it2) {
                std::cout << *it2 << "  ";
            }
            std::cout << std::endl;

            std::map<int, std::string> errorpage = it1->getErrorPage();
            for (std::map<int, std::string>::iterator it2 = errorpage.begin(); it2 != errorpage.end(); ++it2) {
                std::cout << it2->first << " -> " << it2->second << " | ";
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
        visite_directive(it, serv);
        servers.push_back(serv);
        std::cout << "++++++++++++++++++++++++ server +++++++++++++++++++++++++++" << std::endl;
    }
    printservs(servers);
}
