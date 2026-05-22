#pragma once
#include <iostream>
#include <string>
#include <curses.h>
#include <initializer_list>

namespace quickchat {
    class Terminal 
    {
    public:
        int ch = 0;
        int namech = 0;
        int curY = 0;
        int y = 0; 
        int x = 0;
        bool hitBorder = false;
        bool startedT = false;
        std::string userName = "";

        WINDOW *inputBorder;
        WINDOW *msgInput;
        WINDOW *msgView;
        WINDOW *optsView;
        WINDOW *viewBorder;
        WINDOW *nameScreen;
        WINDOW *inputBox;

        Terminal(){
            initscr();
            start_color();

            if (!has_colors()) {
                erase();
                refresh();
                endwin();
                std::cout << "Terminal does not support color, quitting" << std::endl;
            }

            startedT = true;
            init_pair(2, COLOR_GREEN, COLOR_BLACK);
            init_pair(1, COLOR_CYAN, COLOR_BLACK);
            init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(4, COLOR_YELLOW, COLOR_BLACK);
            init_pair(5, COLOR_BLUE, COLOR_BLACK);
        }

        ~Terminal(){
            termClose();
        }

        void start(){            
            viewBorder = newwin(21, 52, 1, 0);
            inputBorder = newwin(12, 52, 22, 0);
            optsView = newwin(3, 52, 34, 0);
            msgInput = derwin(inputBorder, 10, 50, 1, 1); //x & y -2
            msgView = derwin(viewBorder, 19, 50, 1, 1);
            refresh();

            //defining View borders
            wattron(viewBorder, COLOR_PAIR(2));
            box(viewBorder, 0, 0);
            wattroff(viewBorder, COLOR_PAIR(2));

            wattron(optsView, COLOR_PAIR(2));
            box(optsView, 0, 0);
            wattroff(optsView, COLOR_PAIR(2));

            //defining Text Borders
            wattron(inputBorder, COLOR_PAIR(2));
            box(inputBorder, ACS_VLINE, (int)' ');
            wattroff(inputBorder, COLOR_PAIR(2));

            //defining Options box
            wattron(optsView, COLOR_PAIR(1));
            mvwprintw(optsView, 1, 1, ":q [Quit]");
            wattroff(optsView, COLOR_PAIR(1));
            wattron(optsView, COLOR_PAIR(3));
            mvwprintw(optsView, 1, 11, ":p [Ping]");
            wattroff(optsView, COLOR_PAIR(3));
            wattron(optsView, COLOR_PAIR(4));
            mvwprintw(optsView, 1, 21, ":c [Clear]");
            wattroff(optsView, COLOR_PAIR(4));
            wattron(optsView, COLOR_PAIR(5));
            mvwprintw(optsView, 1, 32, ":l [Player List]");
            wattroff(optsView, COLOR_PAIR(5));

            //def Text box
            nodelay(msgInput, true);
            keypad(msgInput, true);
            noecho();
            wprintw(msgInput, "> ");

            wrefresh(viewBorder);
            wrefresh(inputBorder);
            wrefresh(optsView);
            wrefresh(msgInput);
        }

        //Function for getting and storing username
        void getName(){
            nameScreen = newwin(37, 52, 0, 0);
            nodelay(nameScreen, false);
            keypad(nameScreen, true);
            noecho();

            wattron(nameScreen, COLOR_PAIR(2));
            box(nameScreen, 0, 0);
            mvwprintw(nameScreen, 12, 18, "Enter a Username");
            wattroff(nameScreen, COLOR_PAIR(2));
            wrefresh(nameScreen);

            wattron(nameScreen, COLOR_PAIR(2));
            mvwprintw(nameScreen, 15, 12, "___________________________");
            wattroff(nameScreen, COLOR_PAIR(2));
            wrefresh(nameScreen);

            inputBox = derwin(nameScreen, 1, 20, 14, 16);
            wmove(nameScreen, 14, 16); //moving to inputbox
            wrefresh(inputBox);
            wrefresh(nameScreen);
        };

        void clear(std::initializer_list<WINDOW*> winList){
            for (WINDOW* win: winList){
                delwin(win);
            }
            erase();
            refresh();
        };

        void prInput(const char *str){ //reprints the whole input box
            wclear(msgInput);
            wprintw(msgInput, "> %s", str);
            wrefresh(msgInput);
        }

        void prText(const char *str){ //prints to the chat log
            wprintw(msgView, "%s", str);
            curY ++;
            wrefresh(msgView);
            wmove(msgView, curY, 0);    
        }

        void termClose(){
            if (startedT){
                delwin(inputBorder);
                delwin(viewBorder);
                delwin(optsView);
                delwin(msgInput);
                delwin(msgView);
                erase();
                refresh();
                endwin();
                
                startedT = false;
            }
        }
    };
}


/*Will need a start function to greet
will need a funciton to print out chat list
will need a funciton to be called while client is connecting
will need a function to efficnet update client list
will need a way to essientially always need input
will need to always have cmd options on screen8*/