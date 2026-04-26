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
    short LMAX = 0;
    short RMAX = 49;
    short YMAX = 9;

    //need to check for input
    client.Connect("REMOVED_SECRET", REMOVED_SECRET); //doesnt keep the client connected
    std::string text = "";

    getyx(term.msgInputInner, term.y, term.x);
    // term.prText(std::string(
    //     "x val:" + std::to_string(term.x) +
    //     ", text len:" + std::to_string(text.length()) + 
    //     ", y val:" + std::to_string(term.y)
    // ).c_str()); //init with info

    bool bQuit = false;
    while (!bQuit){

        term.ch = wgetch(term.msgInputInner); //allows for user input, gets the char(int) typed
        int pastRows = term.y - 1;
        if (term.ch != ERR){
            if (term.ch == '\n'){ //Handles when user presses enter

                //checking for commands
                if (text == ":q"){
                    bQuit = true;
                    client.Disconnect();
                    term.termClose();
                } else if (text == ":p"){
                    client.PingServer();

                } else if (text == ":c"){
                    wclear(term.msgView);
                    wattron(term.msgView, COLOR_PAIR(2));
                    box(term.msgView, 0, 0);
                    wattroff(term.msgView, COLOR_PAIR(2));
                    wmove(term.msgView, 1, 1);
                    wrefresh(term.msgView);
                    term.curY = 1;
                } else {
                    //prints word to chatbox, reset text and reset input
                    term.prText(text.c_str());
                    text = "";
                    term.prInput(text.c_str());
                }

            } else if (term.ch == '\b'){ //Handles Backspace
                getyx(term.msgInputInner, term.y, term.x);

                if (term.y == 0 && term.x <= 2) { //cursor is at front
                    continue;  

                } else if ((term.y == 0 && term.x - 2 == text.length()) || (48 + 50*pastRows + term.x == text.length()) ){ //cursor at the end
                    text.pop_back();
                    term.prInput(text.c_str());
                  
                } else { //cursor inbetween text 
                    if (term.y == 0){
                        text.erase(term.x-3, 1); //the -2 for curPos -1 for correct index
                        term.prInput(text.c_str());
                        wmove(term.msgInputInner, term.y, term.x-1);

                    } else { //term.y != 0
                        int curPos = 48 + 50*pastRows + term.x;
                        text.erase(curPos-1, 1);
                        term.prInput(text.c_str());
                        if (term.x == LMAX){
                            wmove(term.msgInputInner, term.y-1, RMAX);
                        } else{
                            wmove(term.msgInputInner, term.y, term.x-1);
                        }
                    }
                }

            } else if (term.ch == KEY_LEFT){
                getyx(term.msgInputInner, term.y, term.x);
                
                //bounds check
                if (term.y == 0 && term.x <= 2){
                    continue;
                } else if (term.y != 0 && term.x == LMAX){
                    wmove(term.msgInputInner, term.y-1, 49);
                } else {
                    wmove(term.msgInputInner, term.y, term.x-1);
                }

            } else if (term.ch == KEY_RIGHT){
                getyx(term.msgInputInner, term.y, term.x);

                //Cant move past end of string
                if (term.x == 49 && term.y == YMAX){ //true border
                    continue;
                } else if (term.x == 49){ 
                    wmove(term.msgInputInner, term.y+1, 0);
                    term.y++; //doesn't register yet
                } else if (term.x -2 <= text.length() && term.y == 0){ 
                    wmove(term.msgInputInner, term.y, term.x+1);
                } else if (term.y != 0 ){
                    if (48 + 50*pastRows + term.x <= text.length()){
                        wmove(term.msgInputInner, term.y, term.x+1);
                    }
                }

            } else if ((term.x >= 48 && term.y == YMAX) || term.hitBorder == true){ //49th space reserved for cursor
                if (term.x >= 48 && term.y == YMAX) term.hitBorder = true;
                continue;


            }else { //ALLOW INSERT INBETWEEN TEXT
                getyx(term.msgInputInner, term.y, term.x);
                text += term.ch;
                term.prInput(std::string(text).c_str());

                // if ((term.y == 0 && term.x >= 48) || term.y == 1) {
                //     term.prText(std::string(
                //         "x val:" + std::to_string(term.x) + //+1 accounts for the val we just added
                //         ", text len:" + std::to_string(text.length()) + 
                //         ", cursor pos?:" + std::to_string(48 + (term.x))
                //     ).c_str());
                // }
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
