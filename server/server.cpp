#include <iostream>
#include "server_net.hpp"

enum class MsgIDs : uint32_t {
    ServerPing,
    ServerAccept
};

class Server : public quickchat::serverInterface<MsgIDs>
{
public:
    Server(uint16_t port) : quickchat::serverInterface<MsgIDs>(port){

    }

protected:
    virtual bool OnClientConnect(std::shared_ptr<quickchat::connection<MsgIDs>> client){
        quickchat::message<MsgIDs> msg;
        msg.header.id = MsgIDs::ServerAccept;
        client->Send(msg);
        std::cout << "Sent Connection Message to " << client->GetID() << '\n';
        return true;
    }   

    virtual void OnClientDisconnect(std::shared_ptr<quickchat::connection<MsgIDs>> client){
        std::cout << "Removing client [" << client->GetID() << "]\n";
    }

};


int main(){
    Server server(REMOVED_SECRET);
    server.Start();

    while (1){
        server.Update(-1, true);
    }

    return 0;
}