#pragma once
#include "core.hpp"

//Vector of bytes that will be sent across networks
//Will be used to send data into the connection dequeues

namespace quickchat
    {
        //Foward declare connection
        template <typename ID>
        class connection;

        //Foward declare msgWrapper
        template <typename ID, quickchat::Owner parent>
        struct msgWrapper;

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
                os << "ID: " << int(msg.header.id) << " Msg-Head Size:" << msg.header.size;
                return os;
            }

            template<quickchat::Owner parent>
            void appendText(msgWrapper<ID, parent>& writer, const std::string& text){
                uint32_t textLen = text.length();
                //first push in text in reverse order
                for (uint32_t x=0;x<textLen;x++){
                    writer << text[textLen-x-1]; //-1 for null term
                }
                // push in # of chars last
                writer << textLen;
            }

            template<quickchat::Owner parent>
            std::string& recvText(msgWrapper<ID, parent>& writer, std::string& str){
                uint32_t textLen;
                char val;
                writer >> textLen;

                for (uint32_t x=0;x<textLen;x++){
                    writer >> val;
                    str += val;
                } 
                return str;
            }

            template<quickchat::Owner parent>
            void appendChatList(msgWrapper<ID, parent>& writer, std::vector<std::string> chatList){
                size_t vectLen = chatList.size();
                for (size_t x=0;x<=vectLen-1;++x){
                    writer.msg.appendText(writer, chatList[vectLen-x-1]);
                }
                writer << vectLen;
            }

            template<quickchat::Owner parent>
            std::vector<std::string>& recvChatList(msgWrapper<ID, parent>& writer, std::vector<std::string>& newList){
                size_t vectLen;
                std::string text;

                writer >> vectLen;
                
                for (size_t x=0;x < vectLen; x++){
                    text = writer.msg.recvText(writer, text);
                    newList.push_back(text);
                    text.clear();
                }
                return newList;
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
         

        //Operation for pulling out from the end of an vector
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
       
        //Store client with message
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