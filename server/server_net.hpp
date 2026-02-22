#pragma once
#include "core.hpp"

namespace quickchat
{
    template<typename ID>
    class serverInterface
    {
        public:
            serverInterface(uint16_t port)
            : asioAccepter(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
        {
            
        }

        virtual ~serverInterface(){
            Stop();
        }

        bool Start(){

            try {
                //Gives the server some work to do first, then start the thread
                WaitForClientConnection();
                m_thrContext = std::thread([this]() { m_asioContext.run(); });

            } catch (std::exception& e){
                // Something prohibiting server from listening
                std::cerr << "[SERVER] Exception: " << e.what() << '\n';

                return false;
            }

            std::cout << "[SERVER] Started!\n";
            return true;
        }
                
        void Stop(){
            m_asioContext.stop();
            if (m_thrContext.joinable()) m_thrContext.join();
            std::cout << "SERVER STOPPED!\n";
        }

        void WaitForClientConnection(){
            asioAccepter.async_accept(
                [this](std::error_code ec, asio::ip::tcp::socket socket){
                    if (!ec){
                        std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << '\n';

                        //Make conn 
                        std::shared_ptr<connection<ID>> newconn = std::make_shared<connection<ID>>(connection<ID>::owner::server, m_asioContext,
                        std::move(socket), messagesIN);

                        //relies on the true state of onclient connect b4 overidden, might want to change it to a diff func, or smth else
                        if (OnClientConnect(newconn)){ 
                            clientDeque.push_back(std::move(newconn));
                            clientDeque.back()->ConnectToClient(nIDCounter++);

                            std::cout << "[" << clientDeque.back()->GetID() << "] Connection Approved\n";

                        } else {
                            std::cout << "[-----] Connection Denied\n";
                            }
                    } else {
                        std::cout << "[SERVER] New Connection Error " << ec.message() << '\n';
                    }
                WaitForClientConnection();

                });
        }

        void MessageClient(std::shared_ptr<connection<ID>> client, const message<ID>& msg){
            if (client && client->IsConnected()){
                client->Send(msg);
            } else {
                OnClientDisconnect(client);
                client.reset();

                clientDeque.erase(
                    std::remove(clientDeque.begin(), clientDeque.end(), client), clientDeque.end()
                );
            }
        }

        void MessageAllClients(const message<ID>& msg, std::shared_ptr<connection<ID>> IgnoreClient=nullptr){
            bool bInvalidClient = false;
            for (std::shared_ptr<connection<ID>> client : clientDeque){
                if (client && client->IsConnected()){
                    if (client != IgnoreClient){
                        client->Send(msg);
                    }
                } else {
                    OnClientDisconnect(client);
                    client.reset();
                    bInvalidClient = true;
                }
            }

            if (bInvalidClient){
                clientDeque.erase(
                    std::remove(clientDeque.begin(), clientDeque.end(), nullptr), clientDeque.end()
                );
            }
        }

        void Update(size_t maxMessages = -1, bool bWait = false){
            if (bWait) messagesIN.wait();

            size_t messageCnt = 0;
            while (messageCnt < maxMessages && !messagesIN.empty()){
                owned_message<ID> msgStruct = messagesIN.pop_front();

                //Pass to the message handler
                OnMessage(msgStruct.remote, msgStruct.msg);
                messageCnt++;
            }
        }

        virtual void OnClientValidation(std::shared_ptr<connection<ID>> client){

        }

         
        protected:
        virtual void OnMessage(std::shared_ptr<connection<ID>> client, message<ID>& msg){
            switch (msg.header.id){
                case ID::ServerPing:
                {std::cout << "[" << client->GetID() << "]: Pinged Server!\n";
                client->Send(msg);} //sending back message to client
                break;
            }
        }

        virtual bool OnClientConnect(std::shared_ptr<connection<ID>> client){
            //Override in main
            return true;
        }

        virtual void OnClientDisconnect(std::shared_ptr<connection<ID>> client){
            //Override in main
        }

            
        protected:
        tsqueue<owned_message<ID>> messagesIN;

        //Client list
        std::deque<std::shared_ptr<connection<ID>>> clientDeque;

        asio::io_context m_asioContext;
        std::thread m_thrContext;
        asio::ip::tcp::acceptor asioAccepter;
        uint32_t nIDCounter = 10000;
        
    };
}