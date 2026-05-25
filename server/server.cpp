#include <iostream>
#include "server_net.hpp"

class Server : public quickchat::serverInterface<quickchat::MsgIDs>
{
public:
    Server(uint16_t port) : quickchat::serverInterface<quickchat::MsgIDs>(port){

    }

    virtual void OnClientValidated(std::shared_ptr<quickchat::connection<quickchat::MsgIDs>> client){ //Figure how to use both this and OnCLientConnects or get rid of one
        std::cout << "[" << client->GetID() << "]:" << " Validated!\n";
        quickchat::message<quickchat::MsgIDs> msg;
        msg.header.id = quickchat::MsgIDs::ServerAccept;
        client->Send(msg);
    }

protected:
    virtual bool OnClientConnect(std::shared_ptr<quickchat::connection<quickchat::MsgIDs>> client){
        //Need black/white list feature
        return true;
    }   

    virtual void OnClientDisconnect(std::shared_ptr<quickchat::connection<quickchat::MsgIDs>> client){
        std::cout << "Removing client [" << client->GetID() << "]\n";
        client.reset();
    }
};

int main(){

    if (!quickchat::envCheck()){
        return 1; 
    }

    const int port = std::stoi(std::getenv("SERVER_PORT"));
    Server server(port);
    server.Start();

    while (1){
        server.Update(-1, true);
    }

    return 0;
}