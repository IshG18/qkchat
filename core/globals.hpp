#pragma once
#include "common.hpp"

namespace quickchat {
    enum class MsgIDs : uint32_t {
        ServerPing,
        ServerAccept,
        Chat_GetList, 
        Chat_NewMessage, 

    };

    bool envCheck(){
        if (!std::getenv("SERVER_ADDRESS") || !std::getenv("SERVER_PORT")){
            std::cout << "ENV VARS NEED TO BE SET\n";
            Sleep(3000);
            return false; 
        }
        return true;
    }

    enum class Owner {
        server, client
    };
}