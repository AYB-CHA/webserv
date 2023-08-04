#include "CgiResolver.hpp"
#include "../response/HttpResponseException.hpp"
#include "../utils/string.hpp"

CGIResolver::CGIResolver(const std::string &CGI_path,
                         const std::string &CGI_file, HttpRequest &request,
                         Client &client)
    : request(request), client(client), CGI_path(CGI_path), CGI_file(CGI_file) {
    if (!this->validCGI())
        throw HttpResponseException(502);
    this->buildCGIEnv();
    this->runCGI();
}

void CGIResolver::runCGI() {
    if (pipe(this->read_pipes) == -1 || pipe(this->write_pipes) == -1)
        throw HttpResponseException(500);
    write(this->write_pipes[1], this->client.getPostBody().c_str(),
          this->client.getPostBody().length());
    pid_t pid = fork();
    if (pid == -1)
        throw HttpResponseException(500);
    if (pid == 0) {
        if (dup2(this->read_pipes[1], STDOUT_FILENO) == -1 ||
            dup2(this->write_pipes[0], STDIN_FILENO) == -1)
            this->write_CGI_error_output();
        if (close(this->read_pipes[0]) == -1 ||
            close(this->read_pipes[1]) == -1 ||
            close(this->write_pipes[0]) == -1 ||
            close(this->write_pipes[1]) == -1)
            this->write_CGI_error_output();

        const char *bin = this->CGI_path.c_str();
        char *args[] = {(char *)bin, NULL};
        char **env = new char *[this->env.size() + 1];
        env[this->env.size()] = NULL;
        std::map<std::string, std::string>::const_iterator it =
            this->env.begin();

        for (size_t i = 0; it != this->env.end(); it++) {
            env[i++] = (char *)strdup((it->first + "=" + it->second).c_str());
        }
        execve(bin, args, env);
        this->write_CGI_error_output();
    }
    if (close(this->read_pipes[1]) == -1 || close(this->write_pipes[1]) == -1 ||
        close(this->write_pipes[0]) == -1)
        throw HttpResponseException(500);
    if (fcntl(this->read_pipes[0], F_SETFL, O_NONBLOCK) == -1)
        throw HttpResponseException(500);
}

void CGIResolver::buildCGIEnv() {
    this->env["QUERY_STRING"] = this->request.getQueries();
    this->env["REQUEST_METHOD"] = this->request.getMethod();
    this->env["SCRIPT_FILENAME"] = this->CGI_file;
    this->env["CONTENT_TYPE"] = this->request.getHeader("Content-Type");
    this->env["CONTENT_LENGTH"] = this->request.getHeader("Content-Length");
    this->env["PATH_INFO"] = this->request.getEndpoint();
    // Only needed if the cgi is complied with force-cgi-redirect enabled;
    this->env["REDIRECT_STATUS"] = "200";
    for (std::multimap<std::string, std::string>::const_iterator it =
             this->request.getHeaders().begin();
         it != this->request.getHeaders().end(); it++) {
        this->env["HTTP_" + utils::string::toUpperCase(it->first)] = it->second;
    }
}

bool CGIResolver::validCGI() const {
    return !access(this->CGI_path.c_str(), F_OK | X_OK);
}

int CGIResolver::getReadEnd() const { return this->read_pipes[0]; }

void CGIResolver::write_CGI_error_output() {
    write(1,
          "HTTP/1.1 500 Internal Server Error \r\nContent-Type: "
          "text/html\r\n\r\nLimited Resources.\r\n",
          85);
    exit(1);
}

CGIResolver::~CGIResolver() {}