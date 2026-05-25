#pragma once
#include "common.hpp"
#include "message.hpp"
#include "tsqueue.hpp"
#include "connection.hpp"

namespace quickchat {
    enum class MsgIDs : uint32_t {
        ServerPing,
        ServerAccept
    };

    bool envCheck(){
        if (!std::getenv("SERVER_ADDRESS") || !std::getenv("SERVER_PORT")){
            std::cout << "ENV VARS NEED TO BE SET\n";
            Sleep(3000);
            return false; 
        }
        return true;
    }
}

