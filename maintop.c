//gcc maintop.c funTop.c -o pTOP -lncurses
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <dirent.h>
#include <unistd.h>

#include "funtop.h"

int main(){
    initscr();//inicializa pantalla de ncurses
    cbreak();//las teclas se leen directamnete sin esperar "enter"
    noecho();//evita que se imprima lo que se este capturando
    curs_set(0);//oculta el cursor de la terminal

    //DELCARACION DE LAS ESTRUCTURAS QUE SE USARAN 
    BUFFER bufferControl; //PARA MANEJAR EL TEXTO
    SCREEN screenControl; //MANEJA LAS VENTANAS
    CLI cliControl;       //MANEJA EL CLI
    ARCHIVE archive;      //PARA MANEJAR DIRECTORIOS
    PROCESS process;      //PARA MANEJAR EL CONTENIDO DE CADA PROCESO

    //FUNCION QUE SE ENCARGA DE TODA LA EJECUCION
    initial(&cliControl,&bufferControl, &screenControl, &archive, &process);
    
    endwin();//finaliza el ncurses
    return 0;
}