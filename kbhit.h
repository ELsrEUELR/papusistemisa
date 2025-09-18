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



/*#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}*/
