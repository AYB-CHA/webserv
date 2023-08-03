#include "CgiResolver.hpp"
#include "../response/HttpResponseException.hpp"
#include "../utils/string.hpp"
#define CGI_TIMEOUT 1000000
#define CGI_TIMEOUT_CHUNKS 10

CGIResolver::CGIResolver(const std::string &CGI_path,
                         const std::string &CGI_file,
                         HttpResponseBuilder &response, HttpRequest &request,
                         Client &client)
    : request(request), response(response), client(client), CGI_path(CGI_path),
      CGI_file(CGI_file) {
    if (!this->validCGI())
        throw HttpResponseException(502);
    this->buildCGIEnv();
    this->runCGI();
    std::cout << "WE RUN CGI" << std::endl;
    this->response.setStatuscode(200);
}

void CGIResolver::runCGI() {
    // todo: check for errors here.
    pipe(this->read_pipes);
    pipe(this->write_pipes);
    write(this->write_pipes[1], this->client.getPostBody().c_str(),
          this->client.getPostBody().length());

    int pid;
    if (!(pid = fork())) {
        dup2(this->read_pipes[1], STDOUT_FILENO);
        dup2(this->write_pipes[0], STDIN_FILENO);

        close(this->read_pipes[0]);
        close(this->read_pipes[1]);
        close(this->write_pipes[0]);
        close(this->write_pipes[1]);

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
    }
    close(this->read_pipes[1]);
    close(this->write_pipes[1]);
    close(this->write_pipes[0]);

    fcntl(this->read_pipes[0], F_SETFL, O_NONBLOCK);
}

// void CGIResolver::monitorForTimeOut(pid_t pid) {
//     int status;
//     size_t i = 0;
//     for (; i < CGI_TIMEOUT_CHUNKS; i++) {
//         if (waitpid(pid, &status, WNOHANG) == pid)
//             break;
//         usleep(CGI_TIMEOUT / CGI_TIMEOUT_CHUNKS);
//     }

//     if (i == CGI_TIMEOUT_CHUNKS)
//         throw HttpResponseException(504);
// }

void CGIResolver::buildCGIEnv() {
    this->env["QUERY_STRING"] = this->request.getQueries();
    this->env["REQUEST_METHOD"] = this->request.getMethod();
    this->env["SCRIPT_FILENAME"] = this->CGI_file;
    this->env["CONTENT_TYPE"] = this->request.getHeader("Content-Type");
    this->env["CONTENT_LENGTH"] = this->request.getHeader("Content-Length");
    // Only needed if the cgi is complied with force-cgi-redirect enabled;
    this->env["REDIRECT_STATUS"] = "200";
    for (std::multimap<std::string, std::string>::const_iterator it =
             this->request.getHeaders().begin();
         it != this->request.getHeaders().end(); it++) {
        this->env["HTTP_" + utils::string::toUpperCase(it->first)] = it->second;
    }

    // PATH=xx
    // PATH_INFO=xx
    // REQUEST_URI=xx
    // SCRIPT_NAME=xx
}

bool CGIResolver::validCGI() const {
    return !access(this->CGI_path.c_str(), F_OK | X_OK);
}

int CGIResolver::getReadEnd() const { return this->read_pipes[0]; }

CGIResolver::~CGIResolver() {}