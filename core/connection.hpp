#pragma once
#include "core.hpp"

namespace quickchat {

    template<typename ID> //foward declare
    class serverInterface;

    template<typename ID>
    class connection : public std::enable_shared_from_this<connection<ID>>{//passes down shared ptr

    public:
        enum class owner {
            server, client
        };

        connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<ID>>& qIN)
            :m_asioContext(asioContext), m_socket(std::move(socket)), messagesIn(qIN)
        {
            OwnerType = parent;

            //Validation check, need to pre define for server conn
            if (OwnerType == owner::server){
                validationOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
                validationCheck = scramble(validationOut);
            }

        }

        virtual ~connection(){};

        uint32_t GetID() const{
            return id;
        }
        
        //Function to be set to client function thru setter
        std::function<void(const std::string&)> logMsg = nullptr;  //has to take in a member function so no c-style

        void setLogger(std::function<void(const std::string&)> cbFunc){
            logMsg = cbFunc;
        }
        
        void connPrint(const std::string &str, connection<ID>::owner parent){
            if (parent == owner::server){
                std::cout << str << '\n';
            } else {
                if (logMsg){
                    logMsg(str);
                } else {
                    std::cout << "Log msg hasn't been set yet"; //For client this wouldn't technically show...
                }
            }
        }

    private:
        void ReadHeader(){
            asio::async_read(m_socket, asio::buffer(&msgsTempIn.header, sizeof(message_header<ID>)),
            [this](std::error_code ec, size_t length){
                if (!ec){
                    if (msgsTempIn.header.size > 0){
                        msgsTempIn.body.resize(msgsTempIn.header.size);
                        ReadBody();
                    } else {
                        //Messages w/o bodies still get added
                        AddToMessageQueue();
                    }

                } else {
                    connPrint("["+std::to_string(id)+"] Read Header Fail", OwnerType);
                    m_socket.close();
                }
            });
        }

        void ReadBody(){
            asio::async_read(m_socket, asio::buffer(msgsTempIn.body.data(), msgsTempIn.body.size()),
            [this](std::error_code ec, size_t length){
                if (!ec){
                    AddToMessageQueue();
                } else {
                    connPrint("["+std::to_string(id)+"] Read Body Fail", OwnerType);
                    m_socket.close();
                }
            });
        }

        void AddToMessageQueue(){
            if (OwnerType == owner::server){
                messagesIn.push_back({this->shared_from_this(), msgsTempIn});
            } else {
                messagesIn.push_back({nullptr, msgsTempIn});
            }

            ReadHeader(); //keep it primed
        }

        void WriteHeader(){
            asio::async_write(m_socket, asio::buffer(&messagesOut.front().header, sizeof(message_header<ID>)),
            [this](std::error_code ec, size_t length){
                if (!ec){
                    if (messagesOut.front().body.size() > 0){
                        WriteBody();
                    } else {
                        messagesOut.pop_front();

                        //Check
                        if (!messagesOut.empty()){
                            WriteHeader();
                        }
                    }
                } else {
                    connPrint("["+std::to_string(id)+"] Write Header Fail", OwnerType);
                    connPrint(std::string(ec.message()) + " (" + std::to_string(ec.value()) + ")", OwnerType);
                    m_socket.close();
                } 
            });
        }

        void WriteBody(){
            asio::async_write(m_socket, asio::buffer(messagesOut.front().body.data(), messagesOut.front().body.size()),
            [this](std::error_code ec, size_t length){
                if (!ec){
                    messagesOut.pop_front();

                    if (!messagesOut.empty()){
                        WriteHeader();
                    }

                } else {
                    connPrint("["+std::to_string(id)+"] Write Body Fail", OwnerType);
                    m_socket.close();
                }
            });
        }

        //Func to kick random connections
        uint64_t scramble(uint64_t input){
            uint64_t out = input ^ 0x1324;
            return out ^ 0x134;
        }

        void WriteValidation(){
            asio::async_write(m_socket, asio::buffer(&validationOut, sizeof(uint64_t)),
            [this](std::error_code ec, std::size_t length)
            {
             if (!ec){
                if (OwnerType == owner::client){ //client waits
                    ReadHeader();
                }
             } else {
                if (OwnerType == owner::server) {
                    connPrint("["+std::to_string(id)+"]" + " Write Validation error: " + ec.message(), OwnerType);
                } else {
                    connPrint("Write Validation error: " + ec.message(), OwnerType);
                }
                m_socket.close();
             }
            });
        }

        void ReadValidation(quickchat::serverInterface<ID>* server = nullptr){
            asio::async_read(m_socket, asio::buffer(&validationIn, sizeof(uint64_t)),
            [this, server](std::error_code ec, std::size_t length){
                if (!ec){
                    if (OwnerType == owner::server){
                        if (validationIn == validationCheck){
                            //client accepted
                            server->OnClientValidated(this->shared_from_this());
                            ReadHeader();
                        } else {
                            connPrint("["+std::to_string(id)+"]" + " Kicked (Validation Fail)", OwnerType);
                            m_socket.close();
                        }
                    } else {
                        validationOut = scramble(validationIn);
                        WriteValidation();
                    }

                } else {
                    if (OwnerType == owner::server){
                        connPrint("["+std::to_string(id)+"]" +  " Disconnected, Read Validation Err: " + ec.message(), OwnerType);    
                    } else {
                        connPrint("Client Disconnected, Read Validation Err: " + ec.message(), OwnerType);
                    }
                    
                    m_socket.close();
                }
            });
        }

    public:
        void ConnectToClient(quickchat::serverInterface<ID>* server, uint32_t uid = 0){
            if (OwnerType == owner::server){
                if (m_socket.is_open()){
                    id = uid;

                    //Going to immediately start validation for all new connections
                    WriteValidation();
                    ReadValidation(server);
                }
            }
        }

        void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints){
            if (OwnerType == owner::client){
                asio::async_connect(m_socket, endpoints, 
                    [this](std::error_code ec, asio::ip::tcp::endpoint){
                        if (!ec){
                            //Start validation into primimg
                            ReadValidation();
                        }
                    });
            }
        }

        bool IsConnected() const {
            return m_socket.is_open();
        }

        void Disconnect(){
            if (IsConnected()){
                asio::post(m_asioContext, [this](){ m_socket.close(); });
            }
        }

        void Send(const message<ID>& msg){
            asio::post(m_asioContext,
                [this, msg](){
                    //push msg into queue then see if its alr writing

                    bool writingMsg = !messagesOut.empty();
                    messagesOut.push_back(msg);
                    if (!writingMsg){
                        WriteHeader();
                    }
                    
                });
        }
 

    protected:
        asio::ip::tcp::socket m_socket;
        asio::io_context& m_asioContext; //takes a ref to context

        tsqueue<message<ID>> messagesOut;
        tsqueue<owned_message<ID>>& messagesIn; //takes ref to tsqueue
        message<ID> msgsTempIn; //temp msg vars
        message<ID> msgsTempOut;

        owner OwnerType; //defaults to server
        uint32_t id = 0;

        //Handshake validation
        uint64_t validationOut = 0;
        uint64_t validationIn = 0;
        uint64_t validationCheck = 0;            
    };

}
