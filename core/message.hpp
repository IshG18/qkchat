#pragma once
#include "core.hpp"

//Vector of bytes that will be sent across networks
//Will be used to send data into the connection dequeues

namespace quickchat
    {
        template <typename ID>
        struct message_header {
            ID id{};
            uint32_t size = 0;
        };

        template <typename ID>
        struct message {
            message_header<ID> header{};
            std::vector<uint8_t> body; //Makes a buffer (vector of bytes)

            size_t size() const { 
                return body.size();
            }

            // overloads printing of message objects
            friend std::ostream& operator << (std::ostream& os, const message<ID>& msg){
                os << "ID: " << int(msg.header.id) << "Msg-Head Size:" << msg.header.size;
                return os;
            }

            //Pushes POD-like data into the message buffer thru <<
            template<typename DataType>
            friend message<ID>& operator << (message<ID>& msg, DataType& data){ 
                //This ensures that only simple, predictable types, important if the code is doing low-level operations like memory copying
                static_assert(std::is_standard_layout_v<DataType>, "Data is too complex to be pushed");

                size_t i = msg.body.size();

                msg.body.resize(msg.body.size() + sizeof(DataType)); 
    
                memcpy(msg.body.data() + i, &data, sizeof(DataType));

                //Recalcs size
                msg.header.size = msg.size();

                return msg;

            }

            template <typename DataType>
            friend message<ID>& operator >> (message<ID>& msg, DataType& data){
                static_assert(std::is_standard_layout_v<DataType>, "Data is too complex to be pushed"); 

                size_t i = msg.body.size() - sizeof(DataType);

                //Copy the data from the buffer into the user variable
                memcpy(&data, msg.body.data() + i, sizeof(DataType));

                msg.body.resize(i);

                msg.header.size = msg.size();

                return msg;
            }
        };
    
         //Foward declare the class
        template <typename ID>
        class connection;

        template <typename ID>
        struct owned_message { //inits the shared_ptr but still needs std::make_shared
            std::shared_ptr<connection<ID>> remote = nullptr; 
            message<ID> msg;

            friend std::ostream& operator << (std::ostream& os, const owned_message<ID>& msg){
                os << msg.msg;
                return os;
            }
        };
    }