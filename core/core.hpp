#pragma once
#include "common.hpp"
#include "message.hpp"
#include "tsqueue.hpp"
#include "connection.hpp"

namespace quickchat {
    const std::string host = std::getenv("SERVER_ADDRESS");
    const int port = std::stoi(std::getenv("SERVER_PORT"));
}