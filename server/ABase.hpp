#pragma once
#include <map>
#include <string>
#include <vector>

class ABase {
  protected:
    std::string root;
    std::string upload_path;
    std::map<std::string, bool> allowed_methods;
    std::vector<std::string> index;
    std::map<int, std::string> error_page;
    long client_max_body_size;

  public:
    ABase();
    ABase(const ABase& o);
    virtual ~ABase() = 0;

    const std::string &getRoot(void) const;
    const std::string &getUploadPath(void) const;
    const std::map<std::string, bool> &getAllowedMethods(void) const;
    const std::vector<std::string> &getIndex(void) const;
    const std::map<int, std::string> &getErrorPage(void) const;
    const long &getClientMaxBodySize(void) const;

    void setRoot(std::string root);
    void setUploadPath(std::string upload_path);
    void setAllowedMethods(std::string method);
    void setIndex(std::string index);
    void setErrorPage(int status_code, std::string path);
    void setClientMaxBodySize(long max_body_size);
};
