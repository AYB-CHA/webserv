#pragma once
#include <iostream>

class FormData {
  private:
    int i;
    bool its_file;
    std::string filename;
    std::string content_disposition_value;
    const std::string &to_process;
    const std::string &upload_path;

  public:
    FormData(const std::string &, const std::string &upload_path);
    void processBoundary();
    ~FormData();

  private:
    void processHeaders();
    void processDispositionHeader();
    void uploadFile();
};