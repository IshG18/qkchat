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

        int curY = 1;

        Terminal(){
            initscr();
            start_color();
            int y, x, yBeg, xBeg, yMax, xMax;

            if (!has_colors()) {
                endwin();
                prView("Terminal does not support color, should quit");
            }

            init_pair(2, COLOR_GREEN, COLOR_BLACK);
            init_pair(1, COLOR_CYAN, COLOR_BLACK);
            init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(4, COLOR_YELLOW, COLOR_BLACK);
            
            attron(COLOR_PAIR(2));
            printw("~~~~~~~~~~~~~~ Starting QuickChat ~~~~~~~~~~~~~~");
            attroff(COLOR_PAIR(2));

            msgView = newwin(21, 52, 1, 0);
            msgInput = newwin(5, 52, 22, 0);
            optsView = newwin(3, 52, 27, 0);

            refresh();

            wattron(msgView, COLOR_PAIR(2));
            box(msgView, 0, 0);
            wattroff(msgView, COLOR_PAIR(2));
            wmove(msgView, 1, 1);

            wattron(optsView, COLOR_PAIR(2));
            box(optsView, 0, 0);
            wattroff(optsView, COLOR_PAIR(2));

            wattron(msgInput, COLOR_PAIR(2));
            box(msgInput, ACS_VLINE, (int)' ');
            wmove(msgInput, 1, 1);
            nodelay(msgInput, TRUE);
            wattroff(msgInput, COLOR_PAIR(2));

            wattron(optsView, COLOR_PAIR(1));
            mvwprintw(optsView, 1, 1, ":q [Quit]");
            wattroff(optsView, COLOR_PAIR(1));
            wattron(optsView, COLOR_PAIR(3));
            mvwprintw(optsView, 1, 11, ":p [Ping]");
            wattroff(optsView, COLOR_PAIR(3));
            wattron(optsView, COLOR_PAIR(4));
            mvwprintw(optsView, 1, 21, ":t [Test Print]");
            wattroff(optsView, COLOR_PAIR(4));


            //getyx(optsView, y, x);
            //mvwprintw(msgView, 1, 1, "Y: %d X: %d", y, x);

            wrefresh(msgView);
            wrefresh(msgInput);
            wrefresh(optsView);
           
            ch = wgetch(msgInput);
            wrefresh(msgInput);
        }

        ~Terminal(){
            delwin(msgInput);
            delwin(msgView);
            delwin(optsView);
            endwin(); //Cleanly destroy
        }

        void prInput(const char *str){
            wclear(msgInput);
            wattron(msgInput, COLOR_PAIR(2));
            box(msgInput, ACS_VLINE, (int)' ');
            wattroff(msgInput, COLOR_PAIR(2));
            wmove(msgInput, 1, 1);
            wprintw(msgInput, "> %s", str); //add this to init win
            wrefresh(msgInput);
        }

        void prView(const char *str){
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