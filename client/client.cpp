#include <iostream>
#include "client.hpp"

enum class MsgIDs : uint32_t {
    ServerPing,
    ServerAccept
};

class Client : public quickchat::clientInterface<MsgIDs>
{

};

int main(){
    Client client;
    client.Connect("REMOVED_SECRET", REMOVED_SECRET); //doesnt keep the client connected

    bool bQuit = false;
    while (!bQuit){
        if (client.IsConnected()){
            if (!client.Incoming().empty()){ //Checks for messages
                std::cout << "NEW MESSAGE\n";
                quickchat::message<MsgIDs> msg = client.Incoming().pop_front().msg;

                switch (msg.header.id){
                    case MsgIDs::ServerAccept:
                    {
                        std::cout << "Server accepted connection!\n";
                    }
                    break;
                }
            }
        } else {
            std::cout << "Server Down \n";
            bQuit = true;
        }
    }

    return 0;
}