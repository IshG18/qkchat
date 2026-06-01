#include <iostream>
#include "client_net.hpp"
#include "windows.h"

//Windows C API func to resize console 
void resizeConsole(int rows, int cols){
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    //shrink size so buffer doesnt conflict
    SMALL_RECT temp = {0, 0, 1, 1};
    SetConsoleWindowInfo(hOut, TRUE, &temp);

    COORD bufferSize = { (SHORT)rows, (SHORT)cols };
    SetConsoleScreenBufferSize(hOut, bufferSize);

    SMALL_RECT windowSize = { 0, 0, (SHORT)(rows - 1), (SHORT)(cols - 1) };
    SetConsoleWindowInfo(hOut, TRUE, &windowSize);
}

//Windows C API func to gen console
void createConsole(){
    char fPath[MAX_PATH];
    GetModuleFileNameA(NULL, fPath, MAX_PATH);

    //Windoww's "wishlist"
    STARTUPINFOA si = { sizeof(si)}; //init
    PROCESS_INFORMATION pi;

    //sets new process to child so it doesnt loop & passes down console size
    std::string commandLine = "conhost.exe " + std::string(fPath) + " --spawned"; 

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
        Sleep(100); //avoids race condition
    }
    
    if (!quickchat::envCheck()){
        return 1; 
    }

    quickchat::Terminal term;
    const short LMAX = 0;
    const short RMAX = 49;
    const short YMAX = 9;
    const short textMax = 496;    
    const std::string host = std::getenv("SERVER_ADDRESS");
    const int port = std::stoi(std::getenv("SERVER_PORT"));

    //Login Screen
    term.getName();

    while(true) {
        term.namech = wgetch(term.nameScreen);

        if((term.namech == KEY_ENTER || term.namech == '\n' || term.namech == '\r') && !term.userName.empty()) {
            break;

        } else if(term.namech == KEY_BACKSPACE || term.namech == 127 || term.namech == '\b') {
            if(!term.userName.empty()) {
                term.userName.pop_back();
                mvwprintw(term.inputBox, 0, term.userName.size(), " ");
                wmove(term.nameScreen, 14, 16 + (int)term.userName.size());
                wrefresh(term.nameScreen);
                wrefresh(term.inputBox);
            }

        } else if(term.namech == KEY_LEFT || term.namech == KEY_RIGHT) {
            continue;

        } else if(term.namech != ERR && term.namech >= 32 && term.namech <= 126 && term.userName.size() < 20) {
            term.userName += (char)term.namech;
            mvwprintw(term.inputBox, 0, term.userName.size()-1, "%c", term.namech);
            wmove(term.nameScreen, 14, 16 + (int)term.userName.size());
            wrefresh(term.nameScreen);
            wrefresh(term.inputBox);
            }
    }

    term.userName.erase(std::remove(term.userName.begin(), term.userName.end(), ' '), term.userName.end());
    term.clear({term.nameScreen, term.inputBox});
    term.start();
    quickchat::clientInterface<quickchat::MsgIDs> client(&term);

    //need to check for input
    client.Connect(host, port); //doesnt keep the client connected
    std::string text = "";

    getyx(term.msgInput, term.y, term.x);
    bool bQuit = false;
    while (!bQuit){

        term.ch = wgetch(term.msgInput); //allows for user input
        int pastRows = term.y - 1;
        if (term.ch != ERR){
            if (term.ch == '\n'){ //on enter

                //checking for commands
                if (text == ":q"){
                    bQuit = true;
                    client.Disconnect();
                    term.termClose();
                } else if (text == ":p"){
                    client.PingServer();

                } else if (text == ":c"){
                    wclear(term.msgView);
                    wrefresh(term.msgView);
                    term.curY = 0;
                } else if (text == ":l"){
                    quickchat::message<quickchat::MsgIDs> usrMsg;
                    quickchat::msgWrapper<quickchat::MsgIDs, quickchat::Owner::client> writer{usrMsg};
                    usrMsg.header.id = quickchat::MsgIDs::Chat_GetUsers;
                    client.Send(usrMsg);
                } else {
                    if (!text.empty()){
                        //Send message to server
                        //prints word to chatbox, reset text and reset input
                        quickchat::message<quickchat::MsgIDs> txtMsg;
                        quickchat::msgWrapper<quickchat::MsgIDs, quickchat::Owner::client> writer{txtMsg};
                        txtMsg.header.id = quickchat::MsgIDs::Chat_NewMessage;
                        std::string finalStr = term.userName + ": " + text;
                        txtMsg.appendText(writer, finalStr);
                        client.Send(txtMsg);
                        term.prText(finalStr.c_str());
                        text = "";
                        term.prInput(text.c_str());
                    }
                }

            } else if (term.ch == '\b'){ //Handles Backspace
                getyx(term.msgInput, term.y, term.x);

                if (term.y == 0 && term.x <= 2) { //cursor is at front
                    continue;  

                } else if ((term.y == 0 && term.x - 2 == text.length()) || (48 + 50*pastRows + term.x == text.length()) ){ //cursor at the end
                    text.pop_back();
                    term.prInput(text.c_str());
                    if (term.hitBorder) term.hitBorder = false;
                  
                } else { //cursor inbetween text 
                    if (term.y == 0){
                        text.erase(term.x-3, 1); //the -2 for curPos -1 for correct index
                        term.prInput(text.c_str());
                        wmove(term.msgInput, term.y, term.x-1);
                        if (term.hitBorder) term.hitBorder = false;

                    } else {
                        const int curPos = 48 + 50*pastRows + term.x;
                        text.erase(curPos-1, 1);
                        term.prInput(text.c_str());
                        if (term.x == LMAX){
                            wmove(term.msgInput, term.y-1, RMAX);
                        } else{
                            wmove(term.msgInput, term.y, term.x-1);
                        }
                        if (term.hitBorder) term.hitBorder = false;
                    }
                }

            } else if (term.ch == KEY_LEFT){
                getyx(term.msgInput, term.y, term.x);
                
                //bounds check
                if (term.y == 0 && term.x <= 2){
                    continue;
                } else if (term.y != 0 && term.x == LMAX){
                    wmove(term.msgInput, term.y-1, 49);
                } else {
                    wmove(term.msgInput, term.y, term.x-1);
                }

            } else if (term.ch == KEY_RIGHT){
                getyx(term.msgInput, term.y, term.x);

                //Cant move past end of string
                if (term.x == 49 && term.y == YMAX){ //true border
                    continue;
                } else if (term.x == 49){ 
                    wmove(term.msgInput, term.y+1, 0);
                    term.y++; //doesn't register yet
                } else if (term.x -2 < text.length() && term.y == 0){ 
                    wmove(term.msgInput, term.y, term.x+1);
                } else if (term.y != 0 ){
                    if (48 + 50*pastRows + term.x < text.length()){
                        wmove(term.msgInput, term.y, term.x+1);
                    }
                }

            } else if (text.length() == textMax || term.hitBorder == true){ //border is at x = 48 or len = 496
                if (text.length() == textMax) term.hitBorder = true;
                continue;

            }else if (term.ch == KEY_UP){
                continue;

            }else if (term.ch == KEY_DOWN){
                continue;

            }else {
                getyx(term.msgInput, term.y, term.x);
                const int curPos = 48 + 50*pastRows + term.x;
                if (term.y == 0 && term.x -2 < text.length()){  
                    text.insert(term.x-2, 1, term.ch);
                    term.prInput(std::string(text).c_str());
                    if (term.x == RMAX){
                        wmove(term.msgInput, term.y+1, 0);
                        term.y++;
                        term.x = 0;
                    } else {
                        wmove(term.msgInput, term.y, term.x+1);
                        term.x++;
                    }
                } else if (term.y != 0 && curPos < text.length()){
                    text.insert(curPos, 1, term.ch);
                    term.prInput(std::string(text).c_str());
                    if (term.x == RMAX){
                        wmove(term.msgInput, term.y+1, 0);
                        term.y++;
                        term.x = 0;
                    } else {
                        wmove(term.msgInput, term.y, term.x+1);
                        term.x++;
                    }

                } else {
                    text += term.ch;
                    term.prInput(std::string(text).c_str());
                }
            }
        }

        if (client.IsConnected()){
            if (!client.Incoming().empty()){ //Checks for messages
                quickchat::message<quickchat::MsgIDs> msg = client.Incoming().pop_front().msg;
                quickchat::msgWrapper<quickchat::MsgIDs, quickchat::Owner::client> writer{msg};

                //HANDLE QUOTES AND ESCAPE CHARACTERS
                switch (msg.header.id){
                    case quickchat::MsgIDs::ServerAccept:
                    {
                        term.prText("Server accepted connection!");
                        wclear(term.msgView);
                        wrefresh(term.msgView);
                        term.curY = 0;

                        //Sending back username
                        quickchat::message<quickchat::MsgIDs> userMsg;
                        quickchat::msgWrapper<quickchat::MsgIDs, quickchat::Owner::client> newWriter{userMsg};
                        userMsg.header.id = quickchat::MsgIDs::Chat_NewUser;
                        userMsg.appendText(newWriter, term.userName);
                        client.Send(userMsg);
                    }
                    break;

                    case quickchat::MsgIDs::ServerPing:
                    {
                        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point timeThen;
                        writer >> timeThen;
                        term.prText(std::string("Ping: " + std::to_string(std::chrono::duration<double>(timeNow - timeThen).count())).c_str());
                    }
                    break;

                    case quickchat::MsgIDs::Chat_NewMessage:
                    {
                        std::string message;
                        message = msg.recvText(writer, message);
                        term.prText(message.c_str());
                    }
                    break;

                    case quickchat::MsgIDs::Chat_GetList:
                    {
                        std::vector<std::string> chatList;
                        chatList = msg.recvList(writer, chatList);
                        for (std::string text : chatList){
                            term.prText(text.c_str());
                        }
                    }
                    break;

                    case quickchat::MsgIDs::Chat_GetUsers:
                    {
                        text = "Users: ";
                        std::vector<std::string> usrList;
                        usrList = msg.recvList(writer, usrList);

                        for (std::string str : usrList){
                            if (!(str == usrList[usrList.size()-1])){
                                text += str + ", ";
                            } else {
                                text += str;
                            }                   
                        }
                        term.prInput(text.c_str());
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
