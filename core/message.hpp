#pragma once
#include "core.hpp"

//Vector of bytes that will be sent across networks
//Will be used to send data into the connection dequeues

namespace quickchat
    {
        //Foward declare the class
        template <typename ID>
        class connection;

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

            message<ID>& appendText(message<ID>& msg, std::string& text){
                size_t oldSize = msg.body.size();
                uint32_t textlen = text.length();
                
                //push in # of chars first
                msg.body.resize(msg.body.size() + sizeof(uint32_t));
                memcpy(msg.body.data()+oldSize, &textlen, sizeof(uint32_t));
                
                //push in whole body
                oldSize = msg.body.size();
                msg.body.resize(msg.body.size() + textlen);
                memcpy(msg.body.data()+oldSize, text.data(), textlen);

                msg.header.size = msg.body.size();
                return msg;
            }
        };

        template <typename ID, quickchat::Owner parent>
        struct msgWrapper { //wrapper for passing in owner
            //just a refernce for a msg
            message<ID>& msg;
        };

        //Wrapper funcs to move POD-like data in message buffer thru osstream
        template<typename DataType, typename ID,  quickchat::Owner parent>
        msgWrapper<ID, parent>& operator << (msgWrapper<ID, parent>& writer, DataType& data){
            //This ensures that only simple, predictable types, important if the code is doing low-level operations like memory copying
                if constexpr(std::is_trivially_copyable_v<DataType>){ //was is_standard_layout_v
                    size_t i = writer.msg.body.size();
                    writer.msg.body.resize(writer.msg.body.size() + sizeof(DataType));
                    memcpy(writer.msg.body.data() + i, &data, sizeof(DataType));
                    writer.msg.header.size = writer.msg.size(); //Recalcs size
                    return writer;
                } else {
                    quickchat::connection<ID>::connPrint("Data is too complex to be pushed", parent);
                    return writer;
                }
        }

        template<typename DataType, typename ID, quickchat::Owner parent>
        msgWrapper<ID, parent>& operator >> (msgWrapper<ID, parent>& writer, DataType& data){
            if constexpr(std::is_trivially_copyable_v<DataType>){
                size_t i = writer.msg.body.size() - sizeof(DataType);
                memcpy(&data, writer.msg.body.data() + i, sizeof(DataType)); //Copy data into user variable
                writer.msg.body.resize(i);
                writer.msg.header.size = writer.msg.size();
                return writer;
            } else {
                quickchat::connection<ID>::connPrint("Data is too complex to be pushed into different data type", parent);
                return writer;
            }
        }
       
        //Sends client with clients
        template <typename ID>
        struct owned_message {
            std::shared_ptr<connection<ID>> remote = nullptr; 
            message<ID> msg;

            friend std::ostream& operator << (std::ostream& os, const owned_message<ID>& msg){
                os << msg.msg;
                return os;
            }
        };
    }