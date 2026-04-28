#pragma once
#include <iostream>
#include <string>
#include <curses.h>

namespace quickchat {
    class Terminal 
    {
    public:
        int ch;
        WINDOW *inputBorder;
        WINDOW *msgInput;
        WINDOW *msgView;
        WINDOW *optsView;
        WINDOW *viewBorder;

        int curY = 1;
        int y = 0; int x = 0;
        bool hitBorder = false;

        Terminal(){
            initscr();
            start_color();
            int yBeg, xBeg, yMax, xMax;

            if (!has_colors()) {
                endwin();
                prText("Terminal does not support color, should quit");
            }

            init_pair(2, COLOR_GREEN, COLOR_BLACK);
            init_pair(1, COLOR_CYAN, COLOR_BLACK);
            init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(4, COLOR_YELLOW, COLOR_BLACK); 
            
            attron(COLOR_PAIR(2));
            printw("~~~~~~~~~~~~~~ Starting QuickChat ~~~~~~~~~~~~~~");
            attroff(COLOR_PAIR(2));

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

        ~Terminal(){
            delwin(inputBorder);
            delwin(viewBorder);
            delwin(optsView);
            delwin(msgInput);
            delwin(msgView);
            endwin(); //Cleanly destroy
        }

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
           delwin(inputBorder);
            delwin(viewBorder);
            delwin(optsView);
            delwin(msgInput);
            delwin(msgView);
            endwin();
        }

        void showTxts(){

        }

        void waiting(){

        }

        //needs to handle inout
        //needs to show cmds somehow at bottom
    };
}


/*Will need a start function to greet
will need a funciton to print out chat list
will need a funciton to be called while client is connecting
will need a function to efficnet update client list
will need a way to essientially always need input
will need to always have cmd options on screen8*/


//std::system("powershell -NoProfile -Command Write-Host \"~~~~~~~~~~~~~ Starting QuickChat ~~~~~~~~~~~~~\" -ForegroundColor Green");