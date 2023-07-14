#pragma once

// clang-format off
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include "client/Client.hpp"
#include "server/Server.hpp"
#include "core/Core.hpp"
#include "config/Config.hpp"