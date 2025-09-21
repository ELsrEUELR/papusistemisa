#ifndef KBHIT_H
#define KBHIT_H

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <ncurses.h>

// Versión compatible con ncurses
int kbhit(void)
{
    int ch;
    
    // Usar timeout para hacer getch() no bloqueante temporalmente
    timeout(0);
    ch = getch();
    timeout(-1); // Restaurar comportamiento bloqueante por defecto
    
    if(ch != ERR)
    {
        // Devolver el carácter al buffer de ncurses
        ungetch(ch);
        return 1;
    }
    
    return 0;
}

#endif