#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <dirent.h>
#include <unistd.h>

#include "funtop.h"
#include "kbhit.h"

//initialization functions

void InitializeBuffer(BUFFER *bufferControl){
    bufferControl->c = '\0';
    bufferControl->bufcont = 0;
    bufferControl->buffer[0] = '\0';
    bufferControl->command[0] = '\0';
    bufferControl->parameter1[0] = '\0';
    bufferControl->parameter2[0] = '\0';
}

void InitializeScreen(SCREEN *screenControl){
    screenControl->wind[0] = newwin(3,200,0,0);//buffer
    screenControl->wind[1] = newwin(10,150,3,0);//message
    screenControl->wind[2] = newwin(50,50,15,0);//archive
    screenControl->windupdate[0] = 1;
    screenControl->windupdate[1] = 1;
    screenControl->windupdate[2] = 1;
}

void InitializeArchive(ARCHIVE *arch, SCREEN *screen){
    arch->path = "/proc/";
    arch->dr = opendir(arch->path);
    if (arch->dr == NULL) {
        printMessage(screen, "Could not open current directory");
        exit(1);
    }
}

//

int initial(CLI *cliControl,BUFFER *bufferControl, SCREEN *screenControl, ARCHIVE *archive) {
   
    InitializeBuffer(bufferControl);
    InitializeScreen(screenControl);
    printbuffer(screenControl, bufferControl);
    printMessage(screenControl, "Welcome to top! Press double ENTER or ESC to exit.");
    InitializeArchive(archive, screenControl);
    printArchive(screenControl, archive);

    do{
        windowcontrol(screenControl, bufferControl, archive);
        cli(cliControl,bufferControl, screenControl, archive);
    }while(cliControl->cliState != -1);

    return 0;
}

int cli(CLI *cliControl,BUFFER *bufferControl, SCREEN *screenControl, ARCHIVE *archive) {
    if (kbhit()) {
        cliControl-> cliimput = getch();
        if (cliControl-> cliimput >= 32 && cliControl-> cliimput <= 126 && bufferControl->bufcont < 249) {
            bufferControl->buffer[bufferControl->bufcont] = cliControl-> cliimput;
            bufferControl->bufcont++;
            bufferControl->buffer[bufferControl->bufcont] = '\0';
            screenControl->windupdate[0] = 1;
        }
        else if (cliControl-> cliimput == 127 && bufferControl->bufcont != 0) {
            bufferControl->bufcont--;
            bufferControl->buffer[bufferControl->bufcont] = '\0';
            screenControl->windupdate[0] = 1;
        }
        else if(cliControl-> cliimput == 27) {
            cliControl->cliState = -1;
        }
        else if(cliControl-> cliimput == 10){
            bufferControl->bufcont = 0;
            bufferControl->buffer[0] = '\0';
            screenControl->windupdate[0] = 1;
            screenControl->windupdate[1] = 1;
            screenControl->windupdate[2] = 1;
        }
    }
    return 0;
}

//printing functions

int windowcontrol(SCREEN *screen,BUFFER *buffer, ARCHIVE *arch){
    if(screen->windupdate[0]){
        printbuffer(screen, buffer);
    }
    if(screen->windupdate[1]){
        printMessage(screen, "");
    }
    if(screen->windupdate[2]){
        printArchive(screen, arch);
    }
    return 0;
}

void printbuffer(SCREEN *screen, BUFFER *bufferControl){
    wclear(screen->wind[0]);
    box(screen->wind[0], 0, 0);
    mvwprintw(screen->wind[0],1,1,"-> ");  
    mvwprintw(screen->wind[0],1,3,"%s",bufferControl->buffer);
    screen->windupdate[0] = 0;
    wrefresh(screen->wind[0]);

}

void printMessage(SCREEN *screen, char *message){
    wclear(screen->wind[1]);
    box(screen->wind[1], 0, 0);
    mvwprintw(screen->wind[1],1,1,"%s",message);
    screen->windupdate[1] = 0;
    wrefresh(screen->wind[1]);
}

void printArchive(SCREEN *screen, ARCHIVE *arch){
    wclear(screen->wind[2]);
    int n1=2,n2=1;
    box(screen->wind[2], 0, 0);
    InitializeArchive(arch, screen);
    mvwprintw(screen->wind[2],1,1,"contenido de:%s", arch->path);
    while((arch->di = readdir(arch->dr)) != NULL){
        mvwprintw(screen->wind[2],n1++,1,"%s  %ld",arch->di ->d_name, arch->di ->d_ino);
    }
    screen->windupdate[2] = 0;
    wrefresh(screen->wind[2]);
}