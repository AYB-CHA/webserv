#pragma once
#include <iostream>

class FormData {
  private:
    int i;
    bool its_file;
    std::string filename;
    std::string content_disposition_value;
    const std::string &to_process;

  public:
    FormData(const std::string &);
    void processBoundary();
    ~FormData();

  private:
    void processHeaders();
    void processDispositionHeader();
    void uploadFile();
};