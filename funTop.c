#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <dirent.h>
#include <unistd.h>

#include "funtop.h"
#include "kbhit.h"

//initialization functions

void initializeCLI(CLI *cliControl){
    cliControl-> charsCount = 0;
    cliControl-> cliImput[0] = '\0';
    cliControl-> cliState = 0;
    cliControl-> message[0] = '\0';

}

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
    initializeCLI(cliControl);
    InitializeBuffer(bufferControl);
    InitializeScreen(screenControl);
    InitializeArchive(archive, screenControl);
    do{
        cli(cliControl, bufferControl, screenControl, archive);
        windowcontrol(screenControl, bufferControl, archive, cliControl);
    }while(cliControl->cliState != -1);
    return 0;
}

int cli(CLI *cliControl,BUFFER *bufferControl, SCREEN *screenControl, ARCHIVE *archive) {
    if (kbhit()) {
        cliControl->charsCount = 0;

        do{
            cliControl->cliImput[cliControl->charsCount++] = getch();
        }while(kbhit());

        switch(cliControl->charsCount){
            case 1://keys that return a carter
                //printable characters
                if (cliControl-> cliImput[0] >= 32 && cliControl-> cliImput[0] <= 126 && bufferControl->bufcont < 249) {
                    bufferControl->buffer[bufferControl->bufcont] = cliControl-> cliImput[0];
                    bufferControl->bufcont++;
                    bufferControl->buffer[bufferControl->bufcont] = '\0';
                    screenControl->windupdate[0] = 1;
                    return 1;
                }
                switch (cliControl->cliImput[0]){////switch for control keys
                    case 127://KEY_BACKSPACE
                        if(bufferControl->bufcont != 0){
                            bufferControl->bufcont--;
                            bufferControl->buffer[bufferControl->bufcont] = '\0';
                            screenControl->windupdate[0] = 1;
                            return 1;
                        }
                        break;
                    case 27://KEY_ESCAPE
                        cliControl->cliState = -1;
                        break;
                    case 10://KEY_NEWLINE o ENTER
                        loadComand(screenControl,bufferControl,cliControl);
                        bufferControl->bufcont = 0;
                        bufferControl->buffer[0] = '\0';
                        screenControl->windupdate[0] = 1;
                        screenControl->windupdate[1] = 1;
                        screenControl->windupdate[2] = 1;
                        return 1;
                        break;
                    default:
                        break;
                }
            break;
            case 2:
            break;
            default:
            break;
        }
    }
    return 0;
}

//

void loadComand(SCREEN *sc, BUFFER *bff,CLI *cli){
    if(strcmp(bff->buffer, "exit") == 0){
        cli->cliState = -1;
    }
    else{
        strcpy(cli->message,"invalid command");
        printMessage(sc,cli->message);
    }
}

//printing functions

int windowcontrol(SCREEN *screen,BUFFER *buffer, ARCHIVE *arch,CLI *cC){
    if(screen->windupdate[0]){
        printbuffer(screen, buffer);
    }
    if(screen->windupdate[1]){
        printMessage(screen, cC->message);
    }
    if(screen->windupdate[2]){
        printArchive(screen, arch);
    }
    return 0;
}

void printbuffer(SCREEN *screen, BUFFER *bufferControl){
    werase(screen->wind[0]);
    box(screen->wind[0], 0, 0);
    mvwprintw(screen->wind[0],1,1,"-> ");  
    mvwprintw(screen->wind[0],1,3,"%s",bufferControl->buffer);
    screen->windupdate[0] = 0;
    wrefresh(screen->wind[0]);

}

void printMessage(SCREEN *screen, char *message){
    werase(screen->wind[1]);
    box(screen->wind[1], 0, 0);
    mvwprintw(screen->wind[1],1,1,"%s",message);
    screen->windupdate[1] = 0;
    wrefresh(screen->wind[1]);
}

void printArchive(SCREEN *screen, ARCHIVE *arch){
    werase(screen->wind[2]);
    int n1=2,n2=1;
    box(screen->wind[2], 0, 0);
    rewinddir(arch->dr);
    mvwprintw(screen->wind[2],1,1,"contenido de:%s", arch->path);
    while((arch->di = readdir(arch->dr)) != NULL){
        mvwprintw(screen->wind[2],n1++,1,"%s  %ld",arch->di ->d_name, arch->di ->d_ino);
    }
    screen->windupdate[2] = 0;
    wrefresh(screen->wind[2]);
}