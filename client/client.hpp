#pragma once
#include "core.hpp"

namespace quickchat {
    template <typename ID>
    class clientInterface
    {
        public:
            clientInterface() : context(), socket(context) {}

            virtual ~clientInterface(){
                Disconnect();
            }

            bool Connect(const std::string &host, const uint16_t port){
                try {
                    //Resolve and connect to host
                    asio::ip::tcp::resolver resolver(context);
                    asio::ip::tcp::resolver::results_type endpoints = (resolver.resolve(host, std::to_string(port)));
                    
                    //Initializes connection obj
                    m_connection = std::make_unique<connection<ID>>(
                        connection<ID>::owner::client, context, asio::ip::tcp::socket(context), messagesIn
                    );

                    std::cout << "Starting connection" << '\n';
                    m_connection->ConnectToServer(endpoints);

                    thrContxt = std::thread([this](){context.run();});
                } catch (std::exception &e) {
                    std::cerr << "Client Exception: " << e.what() << '\n';
                    return false;
                }

                return true;
            }

            bool IsConnected(){
                if (m_connection){
                    return m_connection->IsConnected();
                } else {
                    return false;
                }
            }

            void Disconnect(){
                if (IsConnected()){
                    m_connection->Disconnect(); //bit more graceful of a disconnection
                }

                //done with context
                context.stop();
                if (thrContxt.joinable()){
                    thrContxt.join();
                }
            }

        public:
            void Send(const message<ID> &msg){
                if (IsConnected())
                m_connection->Send(msg);
            }

            //Retrieves queue
            tsqueue<owned_message<ID>> &Incoming(){
                return messagesIn;
            }


        protected:
            asio::io_context context;
            std::thread thrContxt;
            asio::ip::tcp::socket socket;
            std::unique_ptr<connection<ID>> m_connection; //doesn't init it

        private:
            //thread safe queue
            tsqueue<owned_message<ID>> messagesIn;

    };
}