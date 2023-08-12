#include "Mime.hpp"

Mime *Mime::self = NULL;

Mime::Mime() {
    // todo: add more...
    this->types["mp4"] = "video/mp4";
    this->types["html"] = "text/html";
    this->types["htm"] = "text/html";
    this->types["css"] = "text/css";
    this->types["xml"] = "text/xml";
    this->types["gif"] = "image/gif";
    this->types["jpeg"] = "image/jpeg";
    this->types["jpg"] = "image/jpeg";
    this->types["js"] = "application/javascript";
    this->types["png"] = "image/png";
    this->types["svg"] = "image/svg+xml";
    this->types["webp"] = "image/webp";
    this->types["ico"] = "image/x-icon";
    this->types["woff"] = "font/woff";
    this->types["woff2"] = "font/woff2";
    this->types["json"] = "application/json";
    this->types["pdf"] = "application/pdf";
    this->types["mp3"] = "audio/mpeg";
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
Mime::~Mime() {
    if (Mime::self != NULL)
        delete Mime::self;
}