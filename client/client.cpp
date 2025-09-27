#include <iostream>
#include "client_net.hpp"

enum class MsgIDs : uint32_t {
    ServerPing,
    ServerAccept
};

int main(){
    quickchat::Terminal term;
    quickchat::clientInterface<MsgIDs> client(&term);

    //need to check for input
    client.Connect("REMOVED_SECRET", REMOVED_SECRET); //doesnt keep the client connected
    std::string text = "";
    std::string cmdstr = "";

    bool bQuit = false;
    while (!bQuit){

        term.ch = wgetch(term.msgInput); //allow for continous str checking
        if (term.ch != ERR){
            if (term.ch == '\n'){
                term.prInput(text.c_str());
                text = "";
            } else {
                text += term.ch;
                //Bounds checking and fixing
            }
            cmdstr = text; //cmd checks CHANGE THIS TO BE AFTER \N
            cmdstr.erase(std::remove(
                cmdstr.begin(), cmdstr.end(), ' '), cmdstr.end());

            if (cmdstr == ":q"){
                client.Disconnect();
                term.termClose();
                bQuit = true;
            } else if (cmdstr == ":t"){
                term.prView("Sent from input");
            } else if (cmdstr == ":p"){
                continue;
            }
        }

        if (client.IsConnected()){
            if (!client.Incoming().empty()){ //Checks for messages
                term.prView("NEW MESSAGE");
                quickchat::message<MsgIDs> msg = client.Incoming().pop_front().msg;

                switch (msg.header.id){
                    case MsgIDs::ServerAccept:
                    {
                        term.prView("Server accepted connection!");
                    }
                    break;
                }
            }
        } else {
            term.prView("Server Down");
            bQuit = true;
        }
    }

    return 0;
}

/*Will need a start function to greet
will need a funciton to print out chat list
will need a funciton to be called while client is connecting
will need a function to efficnet update client list
will need a way to essientially always need input
will need to always have cmd options on screen

*/