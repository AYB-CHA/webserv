#pragma once
#include <map>
#include <string>
#include <vector>
#include <math.h>

class AContext {
  protected:
    std::string root;
    std::string upload_path;
    std::map<std::string, bool> allowed_methods;
    std::vector<std::string> index;
    std::map<int, std::string> error_page;
    long client_max_body_size;
    bool autoindex;

  public:
    AContext();
    AContext(const AContext& o);
    virtual ~AContext() = 0;

    const std::string &getRoot(void) const;
    const std::string &getUploadPath(void) const;
    const std::map<std::string, bool> &getAllowedMethods(void);
    const std::vector<std::string> &getIndex(void);
    const std::map<int, std::string> &getErrorPage(void);
    const long &getClientMaxBodySize(void) const;
    bool getAutoindex(void) const;


    void setRoot(std::string root);
    void setUploadPath(std::string upload_path);
    void setAllowedMethods(std::string method);
    void setIndex(std::string index);
    void setErrorPage(int status_code, std::string path);
    void setClientMaxBodySize(long max_body_size);
    void setAutoindex(bool autoindex);
};
