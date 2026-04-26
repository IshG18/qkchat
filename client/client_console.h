#pragma once
#include <iostream>
#include <string>
#include <curses.h>

namespace quickchat {
    class Terminal 
    {
    public:
        int ch;
        WINDOW *msgInput;
        WINDOW *msgView;
        WINDOW *optsView;
        WINDOW *msgInputInner;

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

            msgView = newwin(21, 52, 1, 0);
            msgInput = newwin(12, 52, 22, 0);
            optsView = newwin(3, 52, 34, 0);
            msgInputInner = derwin(msgInput, 10, 50, 1, 1); //x & y -2

            refresh();

            //defining View box
            wattron(msgView, COLOR_PAIR(2));
            box(msgView, 0, 0);
            wattroff(msgView, COLOR_PAIR(2));
            wmove(msgView, 1, 1);

            wattron(optsView, COLOR_PAIR(2));
            box(optsView, 0, 0);
            wattroff(optsView, COLOR_PAIR(2));

            //defining Borders
            wattron(msgInput, COLOR_PAIR(2));
            box(msgInput, ACS_VLINE, (int)' ');
            wattroff(msgInput, COLOR_PAIR(2));

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
            nodelay(msgInputInner, true);
            keypad(msgInputInner, true);
            noecho();
            wprintw(msgInputInner, "> ");
            

            wrefresh(msgView);
            wrefresh(msgInput);
            wrefresh(optsView);
            wrefresh(msgInputInner);
        }

        ~Terminal(){
            delwin(msgInput);
            delwin(msgView);
            delwin(optsView);
            delwin(msgInputInner);
            endwin(); //Cleanly destroy
        }

        void prInput(const char *str){ //reprints the whole input box
            wclear(msgInputInner);
            wprintw(msgInputInner, "> %s", str);
            wrefresh(msgInputInner);
        }

        void prText(const char *str){ //prints to the chat log
            wprintw(msgView, "%s", str);
            curY ++;
            wrefresh(msgView);
            wmove(msgView, curY, 1);
            
        }

        void termClose(){
            delwin(msgInput);
            delwin(msgView);
            delwin(optsView);
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