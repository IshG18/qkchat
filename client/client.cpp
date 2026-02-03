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

            } else if (term.ch == '\b' && text != ""){ //Handles pressing backspace |  FNEEDS A BOUNDRARY CHECK
                getyx(term.msgInput, term.y, term.x);
                //term.prText(std::string("val: " + std::to_string(term.x)).c_str());
                
                if (term.x - 2 == text.length()){ //cusor is at the end
                    text.pop_back();
                    term.prInput(text.c_str()); //it's easier to just redraw the whole box, instead of mvwdelc
                    
                } else if (term.x -2 < text.length() && term.x > 2){ //cursor isn't at end of text, and not out of bounds
                    text.erase(term.x-3, 1); //the cursor is one behind of what user does, so -3
                    term.prInput(text.c_str());
                    wmove(term.msgInput, term.y, term.x);
                }

                
            } else if (term.ch == KEY_LEFT){
                wmove(term.msgInput, getcury(term.msgInput), getcurx(term.msgInput)-1);

            } else if (term.ch == KEY_RIGHT){
                if (!(getcurx(term.msgInput) -3 >= text.length())){ //user cant move past the end of the string
                    wmove(term.msgInput, getcury(term.msgInput), getcurx(term.msgInput)+1);
                }
                
            } else {
                //Bounds checking and fixing && 
                //Might have to move bounds checks outside it its own if so it can stop arrow movemnts and backspace?
                //Or just make a function to reuuse
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

/*
will need a funciton to print out chat list
will need a function to efficnet update client list
*/