#ifndef _FUNCTOP_H
#define _FUNCTOP_H

#include <ncurses.h>
#include <dirent.h>

typedef struct cli{
    int cliState;
    char cliimput;
}CLI;

typedef struct bff{
    char c;
    char buffer[250];
     int bufcont;
    char parameter1[50];
    char parameter2[50];
    char command[50];
}BUFFER;

typedef struct screen{
    WINDOW* wind[5];
    int windupdate[5];
}SCREEN;

typedef struct archive{
    DIR *dr;
    struct dirent *di;
    const char *path;
}ARCHIVE;

int initial(CLI *cliControl,BUFFER *bufferControl, SCREEN *screenControl, ARCHIVE *archive);
int cli(CLI *cliControl,BUFFER *bufferControl,SCREEN *screenControl, ARCHIVE *archive);

//

void InitializeBuffer(BUFFER *bufferControl);
void InitializeScreen(SCREEN *screenControl);
void InitializeArchive(ARCHIVE *arch, SCREEN *screen);

//
int windowcontrol(SCREEN *screen,BUFFER *buffer, ARCHIVE *arch);
void printbuffer(SCREEN *screens, BUFFER *bufferControl);
void printMessage(SCREEN *screen, char *message);
void printArchive(SCREEN *screen, ARCHIVE *arch);
#endif