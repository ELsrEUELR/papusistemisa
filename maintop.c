#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <dirent.h>
#include <unistd.h>

#include "funtop.h"

int main(){
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    //keypad(stdscr, TRUE);
    BUFFER bufferControl;
    SCREEN screenControl;
    CLI cliControl;
    ARCHIVE archive;
    initial(&cliControl,&bufferControl, &screenControl, &archive);
    
    endwin();
    return 0;
}