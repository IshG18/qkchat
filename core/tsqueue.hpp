//A double linked list that will have to properly managed with thread safety
//Will also hold the message structs
#pragma once
#include "core.hpp"

namespace quickchat {
    template<typename ID>
    class tsqueue {
        public: 
            tsqueue() = default;
            tsqueue(const tsqueue<ID>&) = delete;
            virtual ~tsqueue() { clear(); }

        
        public:
            const ID& front(){
                std::scoped_lock lock(muxQueue);
                return deQueue.front();
            }

            const ID& back(){
                std::scoped_lock lock(muxQueue);
                return deQueue.back();
            }

            ID pop_back(){
                std::scoped_lock lock(muxQueue);
                ID msg = std::move(deQueue.back());
                deQueue.pop_back();
                return msg;
            }

            ID pop_front(){
                std::scoped_lock lock(muxQueue);
                ID msg = std::move(deQueue.front());
                deQueue.pop_front();
                return msg;
            }

            void push_back(const ID& msg){
                std::scoped_lock lock(muxQueue);
                deQueue.emplace_back(std::move(msg));

                std::unique_lock<std::mutex> ul(muxBlocking);
                cvBlocking.notify_one();
                //wake up sleeping thread thats waiting connected to ul
            }

            void push_front(const ID& msg){
                std::scoped_lock lock(muxQueue);
                deQueue.emplace_front(std::move(msg));

                std::unique_lock<std::mutex> ul(muxBlocking);
                cvBlocking.notify_one();
            }

            bool empty(){
                std::scoped_lock lock(muxQueue);
                return deQueue.empty();
            }

            void clear(){
                std::scoped_lock lock(muxQueue);
                deQueue.clear();
            }

            //Stop race coditions
            void wait(){
                while(empty()){
                    std::unique_lock<std::mutex> ul(muxBlocking);
                }
            }

        protected:
            std::mutex muxQueue;
            std::deque<ID> deQueue;
            std::condition_variable cvBlocking;
            std::mutex muxBlocking;

    };
}