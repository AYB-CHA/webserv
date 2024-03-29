#include <cerrno>
#include <cstdlib>
#include <string>

#include "../config/BlockDirective.hpp"
#include "../config/Directive.hpp"

#include "../server/Location.hpp"
#include "../server/Server.hpp"
#include "../server/utils.hpp"

void root_check(std::vector<std::string> params, AContext &base) {
    if (params.size() != 1)
        throw std::runtime_error("Configuration error.");
    base.setRoot(params.front());
}

void upload_path_check(std::vector<std::string> params, AContext &base) {
    if (params.size() != 1)
        throw std::runtime_error("Configuration error.");
    base.setUploadPath(params.front());
}

void accepted_methods_check(std::vector<std::string> params, AContext &base) {
    for (std::vector<std::string>::iterator it = params.begin();
         it != params.end(); ++it) {
        if (*it != "GET" && *it != "POST" && *it != "DELETE")
            throw std::runtime_error("Configuration error.");
        base.setAllowedMethods(*it);
    }
}

void index_check(std::vector<std::string> params, AContext &base) {
    for (std::vector<std::string>::iterator it = params.begin();
         it != params.end(); ++it) {
        base.setIndex(*it);
    }
}

void error_page_check(std::vector<std::string> params, AContext &base) {
    for (std::vector<std::string>::iterator it = params.begin();
         it != params.end(); ++it) {
        std::vector<std::string> s = utils::split(*it, ":");
        if (s.size() != 2 || utils::toInt(s[0]) < 0)
            throw std::runtime_error("Configuration error.");
        else {
            base.setErrorPage(utils::toInt(s[0]), s[1]);
        }
    }
}

bool valid_size(std::string str) {
    char last_char = *(str.rbegin());
    if (last_char != 'M' && last_char != 'K' && last_char != 'G' &&
        last_char != 'B')
        return false;
    for (size_t i = 0; i < str.size() - 1; ++i) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

void client_max_body_size_check(std::vector<std::string> params,
                                AContext &base) {
    if (params.size() != 1 || !valid_size(params.front()))
        throw std::runtime_error("Configuration error.");
    char *end_ptr;
    long size = std::strtol(params.front().c_str(), &end_ptr, 10);
    if (errno == ERANGE || size == 0)
        throw std::runtime_error("Configuration error.");
    switch (*(params.front().rbegin())) {
    case 'G':
        size *= pow(2, 30);
        break;
    case 'M':
        size *= pow(2, 20);
        break;
    case 'K':
        size *= pow(2, 10);
        break;
    default:
        break;
    }
    base.setClientMaxBodySize(size);
}

bool valid_ip_addr(std::string str) {
    std::vector<std::string> s = utils::split(str, ".");
    if (s.size() != 4)
        return false;
    if (utils::toInt(s[0]) > 255 || utils::toInt(s[0]) < 0 ||
        utils::toInt(s[1]) > 255 || utils::toInt(s[1]) < 0 ||
        utils::toInt(s[2]) > 255 || utils::toInt(s[2]) < 0 ||
        utils::toInt(s[3]) > 255 || utils::toInt(s[3]) < 0)
        return false;
    return true;
}

void listen_check(std::vector<std::string> params, AContext &base) {
    for (std::vector<std::string>::iterator it = params.begin();
         it != params.end(); ++it) {
         std::vector<std::string> s;
         std::string::size_type colon = it->find(":");
        if (colon == std::string::npos)
            s.push_back(*it);
        else {
            s.push_back(it->substr(0, colon));
            s.push_back(it->substr(colon + 1));
        }
        if (s.size() == 1) {
            int port = utils::toInt(s.front());
            if (port > 65535 || port < 0)
                throw std::runtime_error("Configuration error.");
            (dynamic_cast<Server &>(base)).setPort(port);
        } else if (s.size() == 2 && valid_ip_addr(s.front())) {
            int port = utils::toInt(s[1]);
            if (port > 65535 || port < 0)
                throw std::runtime_error("Configuration error.");
            (dynamic_cast<Server &>(base)).setPort(port);      // port
            (dynamic_cast<Server &>(base)).setHost(s.front()); // ip addr
        } else {
            throw std::runtime_error("Configuration error.");
        }
    }
}

void server_name_check(std::vector<std::string> params, AContext &base) {
    for (std::vector<std::string>::iterator it = params.begin();
         it != params.end(); ++it) {
        (dynamic_cast<Server &>(base)).setServerName(*it);
    }
}

void redirect_check(std::vector<std::string> params, AContext &base) {
    (dynamic_cast<Location &>(base)).setRedirection(params[0]);
}

void autoindex_check(std::vector<std::string> params, AContext &base) {
    if (params.size() != 1 || (params[0] != "true" && params[0] != "false"))
        throw std::runtime_error("Configuration error.");
    base.setAutoindex(params[0] == "true");
}

void cgi_check(std::vector<std::string> params, AContext &base) {
    for (std::vector<std::string>::iterator it = params.begin();
         it != params.end(); ++it) {
        std::string::size_type colon = it->find(":");
        std::vector<std::string> s;
        if (colon == std::string::npos || colon == 0 || colon == it->size() - 1)
            throw std::runtime_error("Configuration error.");
        s.push_back(it->substr(0, colon));
        s.push_back(it->substr(colon + 1));
        (dynamic_cast<Location &>(base)).setCgiPath(s[0], s[1]);
    }
}

void match_dir(std::vector<Directive>::iterator &it1, AContext &base) {
    std::vector<std::string> params = it1->getParameters();
    switch (it1->getType()) {
    case ROOT:
        root_check(params, base);
        break;
    case UPLOAD_PATH:
        upload_path_check(params, base);
        break;
    case ACCEPTED_METHODS:
        accepted_methods_check(params, base);
        break;
    case INDEX:
        index_check(params, base);
        break;
    case ERROR_PAGE:
        error_page_check(params, base);
        break;
    case CLIENT_MAX_BODY_SIZE:
        client_max_body_size_check(params, base);
        break;
    case LISTEN:
        listen_check(params, base);
        break;
    case SERVER_NAME:
        server_name_check(params, base);
        break;
    case REDIRECT:
        redirect_check(params, base);
        break;
    case AUTOINDEX:
        autoindex_check(params, base);
        break;
    case CGI:
        cgi_check(params, base);
        break;
    default:
        break;
    }
}

void match_loc(Server &serv, std::vector<Directive>::iterator &it1) {
    Location loc;

    loc.setPrefix(it1->getParameters());
    std::vector<Directive> directives1 = it1->getBlock()->getDirectives();
    for (std::vector<Directive>::iterator it2 = directives1.begin();
         it2 != directives1.end(); ++it2)
        match_dir(it2, loc);
    serv.setLocation(loc);
}

void visite_directive(std::vector<Directive>::iterator &it, Server &serv) {
    std::vector<Directive> directives = it->getBlock()->getDirectives();
    for (std::vector<Directive>::iterator it1 = directives.begin();
         it1 != directives.end(); ++it1) {
        switch (it1->getType()) {
        case LOCATION:
            match_loc(serv, it1);
            break;
        default:
            match_dir(it1, serv);
            break;
        }
    }
}

bool unreachableServers(const std::vector<Server>& servers) {
    for (std::vector<Server>::const_iterator outer = servers.begin(); outer != servers.end(); ++outer) {
        for (std::vector<Server>::const_iterator inside = servers.begin(); inside != servers.end(); ++inside) {
            if (outer == inside)
                continue;
            if (outer->getPort() == inside->getPort()) {
                if (outer->getServerNames().empty() || inside->getServerNames().empty())
                    return true;
                for (std::vector<std::string>::const_iterator o = outer->getServerNames().begin(); o != outer->getServerNames().end(); ++o) {
                    for (std::vector<std::string>::const_iterator i = inside->getServerNames().begin(); i != inside->getServerNames().end(); ++i) {
                        if (*o == *i)
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

std::vector<Server> validator(std::vector<Directive> _servers) {

    std::vector<Server> servers;
    for (std::vector<Directive>::iterator it = _servers.begin();
         it != _servers.end(); ++it) {
        Server serv;
        visite_directive(it, serv);
        servers.push_back(serv);
    }
    if (unreachableServers(servers)) {
        std::string errMsg = "Configuration error: unreachable Servers";
        throw std::runtime_error(errMsg);
    }
    return servers;
}
