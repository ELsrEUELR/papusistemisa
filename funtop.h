#ifndef _FUNCTOP_H
#define _FUNCTOP_H

#include <ncurses.h>
#include <dirent.h>

//DECLARACION DE ESTRUCTURAS-----------------------

//ESTRUCTURA PARA CONTROLAR EL CLI
typedef struct cli{
    int cliState;           //estado de la cli
    char cliImput[10];      //entraqda capturada
    int charsCount;         //contador de cararteres
    char namearch[100];     //nombre del archivo a leer
    char message[200];      //mensaje que se imprimira
    char currentFile[200];  //nombre del archivo actual o mostrado
    int row;                //renglon leido
    int maxrow;             //renglines maximos
    char lines[3000][200];  //arreglo para copear el archivo
}CLI;


//ESTRUCTURA PARA EL BUFFER
typedef struct bff{
    char c;             //carater auxiliar
    char buffer[250];   //buffer
    int bufcont;        //contador del buffer
    char parameter1[50];//cadena donde se guerda el parametro 1
    char parameter2[50];//cadena donde se guerda el parametro 2
    char command[50];   //cadena donde se guerda el comando
    int commandstatus;  //variable para saber el numero de caracteres que regresa cada tecla
}BUFFER;

//ESTRUCTURA PARA LAS VENTANAS
typedef struct screen{
    WINDOW* wind[5];        //ARREGLO DE PUNTEROS TIPO VENTANA
    int windupdate[5];      //ARREGLO QUE SIRVE PARA ACTUALIZAR VENTANAS
}SCREEN;

//ESTRUCTURA QUE NOS AYUDA A 
typedef struct archive{
    DIR *dr; //tipo de dato definido en la libreria <dirent>  //representa un directoria abierto
    struct dirent *di; //representa una entrada a un directorio
    const char *path; //un puntero a una cadena >> ruta de directorio
}ARCHIVE;

//ESTRUCTURA PARA GUERDAR LA INFORMACION DE CADA PROCESO
typedef struct process{
    int pid;//ID
    char command[256];//NOMBRE
    long memorySize;//TAMAÑO
} PROCESS;  

//DECLARACION DE FUNCIONES--------------------------------------
//VVVVVVVVVVVVVVVVVVVVVVVVV

////FUNCIONES PARA INICIALIZAR ESTRUCTURAS

void InitializeBuffer(BUFFER *bufferControl);
void InitializeScreen(SCREEN *screenControl);
void InitializeArchive(ARCHIVE *arch, SCREEN *screen);
void initializeCLI(CLI *cliControl);

////FUNCIONES DE CONTROL

int initial(CLI *cliControl,BUFFER *bufferControl, SCREEN *screenControl, ARCHIVE *archive,PROCESS *process);
int cli(CLI *cliControl,BUFFER *bufferControl,SCREEN *screenControl, ARCHIVE *archive);
void loadComand(SCREEN *sc, BUFFER *bff,CLI *cli, ARCHIVE *archive);
void processBuffer(BUFFER *bff);

////FUNCIONES PARA EL MANEJO DEL DIRECTORIO /proc/

int isNumber(const char *str);
void getProcessCommand(int pid, char *command, size_t size);
long getProcessMemorySize(int pid);
void writeProcessInfo(ARCHIVE *arch, SCREEN *screen, const char *filename);

int compareProcesses(const void *a, const void *b);
void writeTopMemoryProcesses(ARCHIVE *arch, SCREEN *screen, const char *filename, int topN);

////FUNCIONES DE IMPRECION

int windowcontrol(SCREEN *screen,BUFFER *buffer, ARCHIVE *arch,CLI *cC);
void printbuffer(SCREEN *screens, BUFFER *bufferControl);
void printMessage(SCREEN *screen, char *message);
void printArchive(SCREEN *screen, ARCHIVE *arch,BUFFER *buffer,CLI *cli);
#endif