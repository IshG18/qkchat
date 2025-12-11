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

    bool bQuit = false;
    while (!bQuit){

        term.ch = wgetch(term.msgInput); //gets the char(int) typed, and updates text
        if (term.ch != ERR){
            if (term.ch == '\n'){ //Handles when user presses enter ||| HOW TO DEAL WITH REGULAR ENTERS?????????

                //checking for commands
                text.erase(std::remove(
                    text.begin(), text.end(), ' '), text.end()
                );

                if (text == ":q"){
                    term.termClose();
                    client.Disconnect();
                    bQuit = true;
                } else if (text == ":t"){
                    term.prView("Sent from input");
                } else if (text == ":p"){
                    continue;
                } else {
                    //prints word to chatbox, resets input, and reset text
                    term.prView(text.c_str());
                    text = "";
                    term.prInput(text.c_str());
                }

            } else {
                //Bounds checking and fixing
                text += term.ch;
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
            term.prView("Disconnected from server!");
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