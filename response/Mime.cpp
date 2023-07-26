#include "Mime.hpp"

Mime *Mime::self = NULL;

Mime::Mime() {
    // todo: add more...
    this->types["mp4"] = "video/mp4";
    this->default_ext = "text/plain";
}

Mime *Mime::getInstance() {
    if (!Mime::self)
        Mime::self = new Mime;
    return Mime::self;
}

const std::string &Mime::getMimeType(const std::string &extention) {
    if (this->types.find(extention) != this->types.end())
        return this->types[extention];
    return this->default_ext;
}