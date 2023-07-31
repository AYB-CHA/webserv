#include "RequestHandler.hpp"
#include "../response/HttpResponseException.hpp"
#include "../response/Mime.hpp"
#include "../utils/string.hpp"
#include <sys/fcntl.h>
#include <sys/stat.h>

#include <cstring>
#include <fstream>
#include <unistd.h> // access

#include <dirent.h>

#include "../server/utils.hpp"

#include <vector>

RequestHandler::RequestHandler(HttpRequest &request, Client &client, std::vector<Server> servers) {
    this->request = request;
    this->client = client;
    fd = -1;
    this->servers = servers;
    handleIt();
}

std::string RequestHandler::getResponse() { return response.build(); }

bool RequestHandler::matchLocation(std::string endpoint, const Server &serv, Location &target) {

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
                if (strcmp(tmp.c_str(), itr1->c_str()) == 0) {
                    found = true;
                    holder = *itr1;
                    target = *itr;
                }
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


// void replace(std::string& holder, std::string s1, std::string s2)
// {
// 	int index = holder.find(s1);
// 	while (index != -1)
// 	{
// 		holder.erase(index, s1.length());
// 		holder.insert(index, s2);
// 		index = holder.find(s1, index + 1);
// 	}
// }


void RequestHandler::handleIt() {

    Server srv;

    std::string hostHeader = request.getHeader("Host");
    srv = validServerName(hostHeader);
    client.setServer(srv);
    std::cout << "updated server: " << srv.getServerNames()[0] << std::endl;



    std::string file = request.getEndpoint();
    std::cout << "end point: " << file << std::endl;

    Location targetLoc;
    bool funStatus = matchLocation(file, client.getServer(), targetLoc);
    if (funStatus) {
        std::string LocationRoot = targetLoc.getRoot();
        if (LocationRoot.empty())
            LocationRoot = client.getServer().getRoot();
        file = "." + LocationRoot + file;
    } else {
        file = "." + client.getServer().getRoot() + file;
    }

    std::cout << "-->file: " << file << std::endl;
    std::cout << "-->function state: " << std::boolalpha << funStatus << std::endl;
    if (access(file.c_str(), F_OK) == -1)
        throw HttpResponseException(404);
    if (access(file.c_str(), R_OK == -1))
        throw HttpResponseException(403);

    struct stat data;
    stat(file.c_str(), &data);

    //check for the existence of the dir
    if (S_ISDIR(data.st_mode)) {
        if (funStatus) {
            std::cout << std::endl << std::endl<< ">> autoindex value: " << targetLoc.getAutoindex() << std::endl;
            if (targetLoc.getAutoindex()) {

                std::string container;
                std::fstream strm("./root/listDir.html", std::ios::in);
                if (!strm.is_open()) {
                    // something
                    std::cout << "error" << std::endl;
                }

                char c;
                while (!strm.eof()) {
                    strm.get(c);
                    container.push_back(c);
                }
                strm.close();

                std::string s1("{content}");
                int index = container.find(s1);
                if (index == -1) {
                    //do somthing
                }
                container.erase(index, s1.length());

                DIR *d = opendir(file.c_str());
                std::cout << "_____________________________________" << std::endl;
                for (dirent *de = readdir(d); de != NULL; de = readdir(d)) {
                    std::string iteam;
                    struct stat info;
                    std::string s(de->d_name);
                    stat(s.c_str(), &info);
                    if (S_ISDIR(info.st_mode)) {
                        if (s == "..")
                            iteam = "\t\t\t<li style=\"list-style-image: url('/images/arrow.png')\"><a href=\"" + s + "\">" + s + "</a>" + "</li>\n";
                        else
                            iteam = "\t\t\t<li style=\"list-style-image: url('/images/folder.png')\"><a href=\"" + s + "\">" + s + "</a>" + "</li>\n";
                    } else {
                        iteam = "\t\t\t<li style=\"list-style-image: url('/images/file.png')\"><a href=\"" + s + "\">" + s + "</a>" + "</li>\n";
                    }

                    container.insert(index, iteam);
                    index += iteam.length();
                }

                response.setStatuscode(200)
                ->setHeader("Content-Type", this->getFileMimeType("test.html"))
                ->pushBody(container);

            } else
                throw HttpResponseException(403);
        } else {

        }
    } else {

        off_t length = data.st_size;

        this->fd = open(file.c_str(), O_RDONLY);
        std::cout << "len: " << length << std::endl;
        // std::cout << "fd: " << fd << std::endl;
        response.setStatuscode(200)
            ->setHeader("Content-Type", this->getFileMimeType(file))
            ->setHeader("Content-Length", utils::string::fromInt(length));
    }

}

int RequestHandler::getFd() { return this->fd; }

std::string
RequestHandler::getFileMimeType(const std::string &file_name) const {
    std::string extention = file_name.substr(file_name.find_last_of('.') + 1);
    return Mime::getInstance()->getMimeType(extention);
}
