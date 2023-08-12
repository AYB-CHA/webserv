#include "FormData.hpp"
#include "../request/RequestHandler.hpp"
#include "../response/HttpResponseException.hpp"
#include "../server/utils.hpp"
#include <fstream>

FormData::FormData(const std::string &to_process,
                   const std::string &upload_path)
    : i(2), its_file(false), to_process(to_process), upload_path(upload_path) {}

void FormData::processHeaders() {
    for (;;) {
        std::string::size_type rc_pos = to_process.find("\r\n", i);
        std::string line = to_process.substr(i, rc_pos - i);

        i += line.length() + 2;

        if (line == "")
            break;
        std::string::size_type column_pos = line.find(':');
        if (column_pos == std::string::npos || line[column_pos + 1] != ' ') {
            throw HttpResponseException(400);
        }
        if (line.substr(0, column_pos) == "Content-Disposition") {
            content_disposition_value = line.substr(2 + column_pos);
        }
    }
}

void FormData::processDispositionHeader() {
    std::vector<std::string> header_parts =
        utils::split(content_disposition_value, ";");
    forEach(std::vector<std::string>, header_parts, it)
        utils::strTrimV2(*it, " ");

    if (header_parts.empty() || header_parts[0] != "form-data")
        throw HttpResponseException(400);
    header_parts.erase(header_parts.begin());
    forEach(std::vector<std::string>, header_parts, it) {
        std::string::size_type equal_pos = it->find('=');
        if (equal_pos == std::string::npos)
            throw HttpResponseException(400);
        std::string key = it->substr(0, equal_pos);
        std::string value = it->substr(equal_pos + 1);
        if (value[0] != '"' || value[value.size() - 1] != '"')
            throw HttpResponseException(400);
        value.pop_back();
        value.erase(value.begin());
        if (key == "filename" && !value.empty()) {
            filename = value;
            its_file = true;
        }
    }
}

void FormData::processBoundary() {
    this->processHeaders();
    this->processDispositionHeader();
    if (its_file)
        this->uploadFile();
}

void FormData::uploadFile() {
    std::string file_path = this->upload_path + "/" + filename;
    std::ofstream upload_file_stream(file_path);
    if (!upload_file_stream)
        throw HttpResponseException(403);
    upload_file_stream.write(to_process.data() + i,
                             to_process.length() - i - 2);
    if (!upload_file_stream)
        throw HttpResponseException(500);
}
FormData::~FormData() {}
