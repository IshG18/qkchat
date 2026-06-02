#pragma once
#include "core.hpp"

namespace quickchat {

    template<typename ID> //foward declare
    class serverInterface;

    template<typename ID>
    class connection : public std::enable_shared_from_this<connection<ID>>{//passes down shared ptr

    public:
        connection(quickchat::Owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<ID>>& qIN)
            :m_asioContext(asioContext), m_socket(std::move(socket)), messagesIn(qIN)
        {
            OwnerType = parent;

            //Validation check, need to pre define for server conn
            if (OwnerType == quickchat::Owner::server){
                svalidationOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
                validationCheck = scramble(svalidationOut);
            }

        }

        virtual ~connection(){};

        uint32_t GetID() const{
            return id;
        }

        static std::function<void(const std::string&)>& getLogger(){ //getter for logMsg
            static std::function<void(const std::string&)> logMsg = nullptr; //Cant be declared static in class, bc of complex variable type 
            return logMsg;
        }

        void setLogger(std::function<void(const std::string&)> cbFunc){
            getLogger() = cbFunc;
        }
        
        static void connPrint(const std::string &str, quickchat::Owner parent){
            if (parent == quickchat::Owner::server){
                std::cout << str << '\n';
            } else {
                if (getLogger()){
                    getLogger()(str); // == auto func getLogger(); func(str); 
                } else {
                    std::cout << str << '\n';
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
            if (OwnerType == quickchat::Owner::server){
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
        std::string scramble(uint64_t input){
            std::string strInput = std::to_string(input);
            //secret would be stored in a real application
            strInput += "58u3CjrD0UYe9j1LlStjmCzM8zV8LXDixRAzd0GcDSHnwNCeo9";
            uint8_t hashBuff[32];
            char hash[64];
            blake3_hasher hasher;
            blake3_hasher_init(&hasher);
            blake3_hasher_update(&hasher, strInput.c_str(), strInput.size());
            blake3_hasher_finalize(&hasher, hashBuff, sizeof(hashBuff));

            for (int i=0;i<32;++i){
                sprintf(hash + i*2, "%02x", hashBuff[i]);
            }
            return std::string(hash);
        }

        void WriteValidation(){
            if (OwnerType == quickchat::Owner::server){
                asio::async_write(m_socket, asio::buffer(&svalidationOut, sizeof(uint64_t)),
                [this](std::error_code ec, std::size_t length)
                {
                    if (ec){
                        connPrint("["+std::to_string(id)+"]" + " Write Validation error: " + ec.message(), OwnerType);
                        m_socket.close();
                    } 
                });
            } else {
                asio::async_write(m_socket, asio::buffer(cvalidationOut.data(), cvalidationOut.size()),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec){
                        ReadHeader();
                    } else {
                        connPrint("Write Validation error: " + ec.message(), OwnerType);
                        m_socket.close();
                    }
                });
            }
        }

        void ReadValidation(quickchat::serverInterface<ID>* server = nullptr){
            if (OwnerType == quickchat::Owner::server){
                asio::async_read(m_socket, asio::buffer(svalidationIn, sizeof(svalidationIn)),
                [this, server](std::error_code ec, std::size_t length){
                    if (!ec){
                        std::string validationStr(svalidationIn, 64);
                        if (validationStr == validationCheck){
                            //client accepted
                            server->OnClientValidated(this->shared_from_this());
                            ReadHeader();
                        } else {
                            connPrint("["+std::to_string(id)+"]" + " Kicked (Validation Fail)", OwnerType);
                            m_socket.close();
                        }

                    } else {
                       connPrint("["+std::to_string(id)+"]" +  " Disconnected, Read Validation Err: " + ec.message(), OwnerType);        
                        m_socket.close();
                    }
                });
            } else {
                asio::async_read(m_socket, asio::buffer(&cvalidationIn, sizeof(uint64_t)),
                [this, server](std::error_code ec, std::size_t length){
                    if (!ec){
                        cvalidationOut = scramble(cvalidationIn);
                        WriteValidation();
                    } else {
                        connPrint("Client Disconnected, Read Validation Err: " + ec.message(), OwnerType);
                        m_socket.close();
                    }
                });
            }
        }

    public:
        void ConnectToClient(quickchat::serverInterface<ID>* server, uint32_t uid = 0){
            if (OwnerType == quickchat::Owner::server){
                if (m_socket.is_open()){
                    id = uid;

                    //Going to immediately start validation for all new connections
                    WriteValidation();
                    ReadValidation(server);
                }
            }
        }

        void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints){
            if (OwnerType == quickchat::Owner::client){
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

        quickchat::Owner OwnerType; //defaults to server
        uint32_t id = 0;

        //Handshake validation
        uint64_t svalidationOut = 0;
        std::string cvalidationOut = "";

        char svalidationIn[64];
        uint64_t cvalidationIn = 0;

        std::string validationCheck = "";            
    };
}
