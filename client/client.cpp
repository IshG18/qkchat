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

        term.ch = wgetch(term.msgInput); //allows for user input, gets the char(int) typed
        if (term.ch != ERR){
            if (term.ch == '\n'){ //Handles when user presses enter

                //checking for commands
                if (text == ":q"){
                    bQuit = true;
                    client.Disconnect();
                    term.termClose();
                } else if (text == ":p"){
                    continue;

                } else {
                    //prints word to chatbox, reset text and reset input
                    term.prText(text.c_str());
                    text = "";
                    term.prInput(text.c_str());
                }

            } else if (term.ch == '\b'){ //Handles pressing backspace
                getyx(term.msgInput, term.y, term.x);
                
                //it's easier to just redraw the whole box, instead of mvwdelch
                text.pop_back();
                term.prInput(text.c_str());

            } else {
                //Bounds checking and fixing
                text += term.ch;
            }
        }

        if (client.IsConnected()){
            if (!client.Incoming().empty()){ //Checks for messages
                term.prText("NEW MESSAGE");
                quickchat::message<MsgIDs> msg = client.Incoming().pop_front().msg;

                switch (msg.header.id){
                    case MsgIDs::ServerAccept:
                    {
                        term.prText("Server accepted connection!");
                    }
                    break;
                }
            }
        } else {
            term.prText("Disconnected from server!");
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