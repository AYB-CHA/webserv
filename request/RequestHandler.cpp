#include "RequestHandler.hpp"
#include "../response/HttpResponseException.hpp"
#include "../cgi/CgiResolver.hpp"
#include "../response/Mime.hpp"
#include "../utils/string.hpp"
#include "../client/Client.hpp"
#include "../selector/Mediator.hpp"
#include <sys/fcntl.h>
#include <sys/stat.h>

#include <cstring>
#include <fstream>
#include <unistd.h> // access

#include <dirent.h>

#include "../server/utils.hpp"

#include <vector>

RequestHandler::RequestHandler() : handled(false) {}

RequestHandler::RequestHandler(const RequestHandler& o)
    : response(o.response), request(o.request),
    servers(o.servers), fd(o.fd), handled(o.handled), file(o.file),
    targetLoc(o.targetLoc), matchLocState(o.matchLocState) {}

RequestHandler::RequestHandler(HttpRequest &request, std::vector<Server>& servers) {
    this->request = request;
    fd = -1;
    this->servers = servers;
}

RequestHandler& RequestHandler::operator=(const RequestHandler& o) {
    if (this == &o) return *this;
    new (this) RequestHandler(o);
    return *this;
}

void RequestHandler::init(Client& client) {
    Server srv;

    std::string hostHeader = request.getHeader("Host");
    srv = validServerName(hostHeader);
    client.setServer(srv);
    client.setMethod(request.getMethod());
    client.setContentLength(utils::string::toInt(request.getHeader("Content-Length")));
}

bool RequestHandler::handlePOST(Client &client, Mediator& mediator) {
    (void)client;
    (void)mediator;
    // if cgi meaning if the extension matches a cgi and we have to go through it
    // if (cgi()) {
    //   if (request.getMethod() == "POST") {
    //    pushStatusLine, headers (chunked transfer encoding) and \r\n\r\n
    //  }
    //  CGI cgi = CGI(path, ext);
    //  client.setCgiFd(pipe[0]);
    //  mediator.addCgi(pipe[0]);
    // } else {
    //  client.setBodyFd(file) // if the file exists etc
    // }
    return true;
}

void RequestHandler::checkConfAndAccess(Client& client) {
    file = request.getEndpoint();
    std::cout << "end point: " << file << std::endl;

    this->matchLocState = matchLocation(file, client.getServer());
    if (this->matchLocState) {
        std::string LocationRoot = this->targetLoc.getRoot();
        if (LocationRoot.empty())
            LocationRoot = client.getServer().getRoot();
        file = "." + LocationRoot + file;
    } else {
        file = "." + client.getServer().getRoot() + file;
    }

    std::cout << "-->file: " << file << std::endl;
    std::cout << "-->function state: " << std::boolalpha << this->matchLocState << std::endl;
    if (access(file.c_str(), F_OK) == -1)
        throw HttpResponseException(404);
    if (access(file.c_str(), R_OK == -1))
        throw HttpResponseException(403);
}

void RequestHandler::createContainer(std::string& container, std::string::size_type& index) {
    std::fstream strm("./root/listDir.html", std::ios::in);
    if (!strm.is_open())
        throw HttpResponseException(500);

    char c;
    while (!strm.eof()) {
        strm.get(c);
        container.push_back(c);
    }
    strm.close();

    std::string s1("{content}");
    index = container.find(s1);
    if (index == std::string::npos) {
        throw HttpResponseException(500);
    }
    container.erase(index, s1.length());
}

void RequestHandler::fillContainer(std::string& container, std::string::size_type& index) {

    DIR *d = opendir(file.c_str());
    std::cout << "_____________________________________" << std::endl;
    for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {
        std::string item;
        std::string s(de->d_name);

        std::cout << "file name: " << s << std::endl;

        if (DT_DIR == de->d_type && s == "..")
            item = std::string("\t\t\t<li style=\"list-style-image: url('/images/arrow.png')\"><a href=\"") + s + "\">" + s + "</a>" + "</li>\n";
        else if (DT_DIR == de->d_type) {
            item = std::string("\t\t\t<li style=\"list-style-image: url('/images/folder.png')\"><a href=\"") + s + "\">" + s + "</a>" + "</li>\n";
        } else {
            item = std::string("\t\t\t<li style=\"list-style-image: url('/images/file.png')\"><a href=\"") + s + "\">" + s + "</a>" + "</li>\n";
        }

        container.insert(index, item);
        index += item.length();
    }
}

void RequestHandler::listDirectory () {
    if (this->matchLocState) {
        std::cout << std::endl << std::endl<< ">> autoindex value: " << this->targetLoc.getAutoindex() << std::endl;
        if (this->targetLoc.getAutoindex()) {

            std::string container;
            std::string::size_type index = 0;

            createContainer(container, index);
            fillContainer(container, index);

            response.setStatuscode(200)
            ->setHeader("Content-Type", this->getFileMimeType("test.html"))
            ->pushBody(container);

        } else
            throw HttpResponseException(403);
    } else
        throw HttpResponseException(404);
}

void RequestHandler::handleGET(Client& client, Mediator& mediator) {
    (void)mediator;

    checkConfAndAccess(client);

    std::cout << "HERE:" << std::endl;

    struct stat data;
    stat(file.c_str(), &data);

    //check for the existence of the dir
    if (S_ISDIR(data.st_mode)) {
        listDirectory();
    } else if (false) {

        off_t length = data.st_size;

        this->fd = open(file.c_str(), O_RDONLY);
        std::cout << "len: " << length << std::endl;
        // std::cout << "fd: " << fd << std::endl;
        response.setStatuscode(200)
            ->setHeader("Content-Type", this->getFileMimeType(file))
            ->setHeader("Content-Length", utils::string::fromInt(length));
    }
    else
    {
        std::cout << "cgi file: "<< file << std::endl;
        CGIResolver cgi("/usr/bin/php-cgi", file, this->response, this->request, client);

        client.setCgiFd(cgi.getReadEnd());
        mediator.addCGI(cgi.getReadEnd());
        return ;
    }

    client.storeResponse(this->getResponse());
    client.setFileFd(this->getFd());
}

std::string RequestHandler::getResponse() { return response.build(); }

bool RequestHandler::matchLocation(std::string endpoint, const Server &serv) {

    std::vector<Location> locations = serv.getLocation();
    if (locations.empty()) {
        return false;
    }

    bool found = false;
    std::string holder = "";

    forEach (std::vector<Location>, locations, itr) {
        forEachConst (std::vector<std::string>, itr->getPrefix(), itr1) {
            std::vector<std::string> list = utils::split(endpoint, "/");
            for (size_t i = 0; i < list.size(); i++) {

                std::string tmp = "";
                for (std::vector<std::string>::iterator itr2 = list.begin(); itr2 != list.begin() + i + 1; itr2++) {
                    tmp += "/" + *itr2;
                }
                // list.pop_back();

                std::cout << "tmp: " << tmp << " | i=  " << i << " | size: " << list.size() << std::endl;

                std::string prefix = *itr1;
                utils::strTrimV2(prefix, "/");
                prefix.insert(prefix.begin(), '/');
                std::cout << "prefix modified: " << prefix << std::endl;
                if (tmp == prefix) {
                    found = true;
                    holder = prefix;
                    this->targetLoc = *itr;
                }
            }

            if (list.empty() && *itr1 == "/") {
                this->targetLoc = *itr;
                return true;
            }
        }
        std::cout << "================ Location ===================" << std::endl;
    }

    return found;
}

Server& RequestHandler::validServerName(std::string serverName) {
    for (std::vector<Server>::iterator itr = servers.begin(); itr != servers.end(); itr++) {
        std::vector<std::string> server_names = itr->getServerNames();
        for (std::vector<std::string>::iterator itr1 = server_names.begin(); itr1 != server_names.end(); itr1++) {
            if (serverName == *itr1) {
                return *itr;
            }
        }
    }
    return *(servers.begin());
}


int RequestHandler::getFd() { return this->fd; }

void    RequestHandler::setInitialized(bool handled) {
    this->handled = handled;
}

bool    RequestHandler::hasBeenInitialized() const {
    return handled;
}

std::string
RequestHandler::getFileMimeType(const std::string &file_name) const {
    std::string extention = file_name.substr(file_name.find_last_of('.') + 1);
    return Mime::getInstance()->getMimeType(extention);
}
