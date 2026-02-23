#include <iostream>
#include "client_net.hpp"

enum class MsgIDs : uint32_t {
    ServerPing,
    ServerAccept
};

//Windows C API func to resize console 
void resizeConsole(int rows, int cols){
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    //Enable Virtual Terminal Processing
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    //Send VT sequence
    std::cout << "\x1b[8;" << cols << ";" << rows << "t" << std::flush;
}

//Windows C API func to gen console
void createConsole(){
    char fPath[MAX_PATH];
    GetModuleFileNameA(NULL, fPath, MAX_PATH);

    //Windoww's "wishlist"
    STARTUPINFOA si = { sizeof(si)}; //init
    PROCESS_INFORMATION pi;

    //sets new process to child so it doesnt loop & passes down console size
    std::string commandLine = std::string(fPath) + " --spawned"; 

    //Spawn a new process of this .exe, then kill this parent process
    if (CreateProcessA(
        NULL, (LPSTR)commandLine.c_str(), NULL, NULL, 
        FALSE, CREATE_NEW_CONSOLE, NULL, NULL,
        &si, &pi
    )
    ){
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        exit(0);
    }
}

int main(int argc, char* argv[]){
    //Check if we are the parent or child process to spawn tui
    if (argc < 2 || std::string(argv[1]) != "--spawned"){
        createConsole();
    } else {
        resizeConsole(52, 37);
        Sleep(200); //avoids race condition
    }

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
                    client.PingServer();

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
                quickchat::message<MsgIDs> msg = client.Incoming().pop_front().msg;

                switch (msg.header.id){
                    case MsgIDs::ServerAccept:
                    {
                        term.prText("Server accepted connection!");
                    }
                    break;

                    case MsgIDs::ServerPing:
                    {
                        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point timeThen;
                        msg >> timeThen;
                        term.prText(std::string("Ping: " + std::to_string(std::chrono::duration<double>(timeNow - timeThen).count())).c_str());
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