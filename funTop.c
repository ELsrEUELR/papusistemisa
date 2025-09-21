#include <stdio.h>      // Librería estándar de C
#include <string.h>     // Funciones para trabajar con cadenas 
#include <stdlib.h>     // Funciones de utilidad general
#include <ncurses.h>    // Librería para manejar interfaces de texto en terminal 
#include <dirent.h>     // Manejo de directorios 
#include <unistd.h>     // Funciones del sistema UNIX
#include <ctype.h>      // Funciones para clasificar y manipular caracteres 

#include "funtop.h"     // Archivo de cabecera propio:contiene prototipos de funciones 
#include "kbhit.h"      // Archivo de cabecera para detectar si una tecla fue presionada sin necesidad de esperar con `getch()`.


//FUNCIONES PARA INICIALIZAR ESTRUCTURAS------------------------------

void initializeCLI(CLI *cliControl){//funcion para inicializar la cli
    cliControl-> charsCount = 0;
    cliControl-> cliImput[0] = '\0';
    cliControl-> cliState = 0;
    cliControl-> message[0] = '\0';
    cliControl->row = 0;
    cliControl->maxrow = 0;

}

void InitializeBuffer(BUFFER *bufferControl){//funcion para inicilaizar el buffer
    bufferControl->c = '\0';
    bufferControl->bufcont = 0;
    bufferControl->buffer[0] = '\0';
    bufferControl->command[0] = '\0';
    bufferControl->parameter1[0] = '\0';
    bufferControl->parameter2[0] = '\0';
    bufferControl->commandstatus = 0;
}

void InitializeScreen(SCREEN *screenControl){//funcion para inicailizar el arreglo de ventanas
    screenControl->wind[0] = newwin(3,200,0,0);  //ventara para mostrar buffer
    screenControl->wind[1] = newwin(7,150,3,0);  //ventana para mostrar mensajes
    screenControl->wind[2] = newwin(34,100,10,0);//ventana para mostrar el archivo
    screenControl->windupdate[0] = 1; 
    screenControl->windupdate[1] = 1;   //variable entera que nos ayuda a actualizar las vantanas 
    screenControl->windupdate[2] = 1;
}

void InitializeArchive(ARCHIVE *arch, SCREEN *screen){  //inicilizar estructura para manejar el directoro
    arch->path = "/proc/";              
    arch->dr = opendir(arch->path);
    if (arch->dr == NULL) {
        printMessage(screen, "no se pudo abrir el directorio");
        exit(1);
    }
}

//FUNCIONES DE CONTROL-----------------------------------------------------------------------------
         
//funcion para poder inicializar todo y el ciclo que estara ejecutando el programa
int initial(CLI *cliControl,BUFFER *bufferControl, SCREEN *screenControl, ARCHIVE *archive,PROCESS *process) {
    //inicializamos todas nuestras struc;
    initializeCLI(cliControl);                 
    InitializeBuffer(bufferControl);
    InitializeScreen(screenControl);
    InitializeArchive(archive, screenControl);

    //ciclo do que matendra el programa en ejecucion hasta no escribir "exit" en el bufer o presionar esq
    //se controla tanto la ejecucion de los comandos como la imprecion de las ventanas
    do{
        cli(cliControl, bufferControl, screenControl, archive);
        windowcontrol(screenControl, bufferControl, archive, cliControl);
    }while(cliControl->cliState != -1);
    return 0;
}


//funcion encargada del estado de la cli,de las entradas al buffer,la actualizacion de las ventans
// y maneja la informacion del directorio con el que va trabajar
int cli(CLI *cliControl,BUFFER *bufferControl, SCREEN *screenControl, ARCHIVE *archive) {    
    if (kbhit()) {     //revisa si hay teclas precionas
        cliControl->charsCount = 0;  //inicilaizamos nuetro contador de caracteres
        
        //ciclo do que nos sirve para verificar que tecla se preciono
        //dependiendo del n de caracteres que nos devuelva la tecla
        do{           
            cliControl->cliImput[cliControl->charsCount++] = getch();
        }while(kbhit());
        cliControl->cliImput[cliControl->charsCount] = '\0';

        //aqui procesamos la entrada dependiendo el numero de caracteres devuelto
        switch(cliControl->charsCount){
            case 1://teclas de un solo valor 
                //caracteres imprimibles
                //verifica que este en el rango de los caracteres
                if (cliControl-> cliImput[0] >= 32 && cliControl-> cliImput[0] <= 126 && bufferControl->bufcont < 249) {
                    bufferControl->buffer[bufferControl->bufcont] = cliControl-> cliImput[0];//agrega el nuevo caracter al buffer
                    bufferControl->bufcont++;//aunmentamos el contador
                    bufferControl->buffer[bufferControl->bufcont] = '\0';
                    screenControl->windupdate[0] = 1;
                    return 1;
                }
                switch (cliControl->cliImput[0]){////switch para teclas de control
                    case 127://KEY_BACKSPACE  "borrar"
                        if(bufferControl->bufcont != 0){
                            bufferControl->bufcont--;
                            bufferControl->buffer[bufferControl->bufcont] = '\0';
                            screenControl->windupdate[0] = 1;
                            return 1;
                        }
                        break;
                    case 27://KEY_ESCAPE "esc"
                        cliControl->cliState = -1;
                        break;
                    case 10://KEY_NEWLINE o ENTER 
                        //al precionar el enter :
                        bufferControl->commandstatus = 0;
                        processBuffer(bufferControl);  //se verifica si se escribio o no un comando
                        loadComand(screenControl,bufferControl,cliControl,archive);//en caso de que si se procesa este
                        printArchive(screenControl,archive,bufferControl,cliControl);//actualizamos la ventana que imprime el dir.
                        //aqui solo regresamos nuestras cadenas a un valor inicial para la siguiente captura 
                        bufferControl->bufcont = 0;
                        bufferControl->buffer[0] = '\0';
                        screenControl->windupdate[0] = 1;
                        screenControl->windupdate[1] = 1;
                        bufferControl->command[0] = '\0';
                        bufferControl->parameter1[0] = '\0';
                        bufferControl->parameter2[0] = '\0';
                        return 1;
                        break;
                    default:
                        break;
                }
            break;
            case 3://teclas que regresan tres valores 
                switch(cliControl->cliImput[2]){//comprando el ultimo valor que regresa
                    case 65://si el ultimo valor es 65 significa que presionamos tecla arriba
                        if(cliControl->row >= 0){
                            //esto es para desplazarce por la ventana que nos muestra los procesos
                            cliControl->row--;
                            printArchive(screenControl,archive,bufferControl,cliControl);
                        }
                    break;
                    case 66://no capta 66 significa tecla abajo
                        if(cliControl->row < cliControl->maxrow){
                            //esto es para desplazarce por la ventana que nos muestra los procesos
                            cliControl->row++;
                            printArchive(screenControl,archive,bufferControl,cliControl);

                        }
                    break;
                } 
            break;
            default:
            break;
        }
    }
    return 0;
}


//funcion que nos separara el bufer en comando parametro1 y parametro2
//en caso de haber ingresado 1 2 o tres palabras
void processBuffer(BUFFER *bff) {
    // Inicializamos por seguridad
    bff->command[0] = '\0';
    bff->parameter1[0] = '\0';
    bff->parameter2[0] = '\0';
    bff->commandstatus = 0;

    char *token = strtok(bff->buffer, " ");  // Primer palabra
    if (token != NULL) {
        strncpy(bff->command, token, sizeof(bff->command) - 1);
        bff->command[sizeof(bff->command) - 1] = '\0'; // Evitar overflow
        bff->commandstatus = 1;
    }

    token = strtok(NULL, " ");               // Siguiente palabra
    if (token != NULL) {
        strncpy(bff->parameter1, token, sizeof(bff->parameter1) - 1);
        bff->parameter1[sizeof(bff->parameter1) - 1] = '\0';
        bff->commandstatus = 2;
    }

    token = strtok(NULL, " ");               // Siguiente palabra
    if (token != NULL) {
        strncpy(bff->parameter2, token, sizeof(bff->parameter2) - 1);
        bff->parameter2[sizeof(bff->parameter2) - 1] = '\0';
        bff->commandstatus = 3;
    }
}


//funcion que procesa los comando escritos
void loadComand(SCREEN *sc, BUFFER *bff, CLI *cli, ARCHIVE *archive) {
    int n;
    //dependiendo de el numero de palabras ingresadas se manejaran los comandos
    switch (bff->commandstatus) {
        case 1: //Solo comando

            if (strcmp(bff->command, "exit") == 0){ //verificamos si escribimos "exit"
                cli->cliState = -1; //en caso de si, poner el estado de la cli para salir
            }
            else if (strcmp(bff->command, "proclist") == 0) {//verificamos si se escribio "proclist" ->comando para lista de procesos
                strncpy(cli->namearch, bff->command, sizeof(cli->namearch) - 1);//copiamos el nombre del comando 
                cli->namearch[sizeof(cli->namearch) - 1] = '\0';
                writeProcessInfo(archive, sc, bff->command);//llamanos a la funcion que se encarga de la lista de procesos
            }
        break;
        case 3: // Comando + 2 parámetros

            if (strcmp(bff->command, "proc") == 0) {//verivicamos si escribimos "proc"

                if (strcmp(bff->parameter1, "topmem") == 0) {//verificamos el primer parametro del comando
                    
                    if (strlen(bff->parameter2) > 0) {//verificamos si ingresamos le tercer parametro
                        n = atoi(bff->parameter2);  // convertir a int
                        strncpy(cli->namearch, bff->parameter1, sizeof(cli->namearch) - 1);
                        cli->namearch[sizeof(cli->namearch) - 1] = '\0';
                        writeTopMemoryProcesses(archive, sc, bff->parameter1, n);
                    } 
                    else {
                        strcpy(cli->message, "ERROR DE SINTAXIS: proc topmem [int]");
                        printMessage(sc, cli->message);
                    }
                } else {
                    strcpy(cli->message, "ERROR: parámetro desconocido");
                    printMessage(sc, cli->message);
                }
            }
        break;

        default:
        break;
    }
}


//FUNCIONES PARA EL MANEJO DEL DIRECTORIO /proc/---------------------------------------------------------

// Función auxiliar para verificar si una cadena es un número
int isNumber(const char *str) {
    if (str == NULL || *str == '\0') return 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}

// Función para obtener el nombre del comando de un proceso
void getProcessCommand(int pid, char *command, size_t size) {
    char path[256];
    FILE *file;
    
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);//en base al id podemos acceder al nombre del proceso
    file = fopen(path, "r");//abrimos el archivo
    
    if (file != NULL) {
        if (fgets(command, size, file) != NULL) {//gurda el nombre(COMMAND)
            // Remover el salto de línea al final
            command[strcspn(command, "\n")] = '\0';
        } else {
            strcpy(command, "unknown");
        }
        fclose(file);
    } else {
        strcpy(command, "unknown");
    }
}

// Función para obtener el tamaño en memoria de un proceso (en kB)
long getProcessMemorySize(int pid) {
    char path[256];
    FILE *file;
    char line[256];
    long vmsize = 0;
    
    snprintf(path, sizeof(path), "/proc/%d/status", pid);//con el pid accedemos el archivo  accedemos a su info. status
    file = fopen(path, "r");
    
    if (file != NULL) {
        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "VmSize:", 7) == 0) {
                // Extraer el valor (ya está en kB)
                sscanf(line, "VmSize: %ld", &vmsize);
                break;
            }
        }
        fclose(file);
    }
    
    return vmsize;
}

//funcion que se enarga de imprimir en un archivo una lista de los procesos 
void writeProcessInfo(ARCHIVE *arch, SCREEN *screen, const char *filename) {
    FILE *outputFile;
    char command[256];
    long memorySize;
    int pid;
    
    // abrir archivo para escritura
    outputFile = fopen(filename, "w");
    if (outputFile == NULL) {
        printMessage(screen, "Error: No se pudo crear el archivo");
        return;
    }
    
    //escribir cabecera
    fprintf(outputFile, "PID      \tCOMMAND\t                                     \tSIZE(kB)\n");
    fprintf(outputFile, "======   \t=======\t                                     \t==========\n");
    //reiniciar el directorio para leer desde el principio
    rewinddir(arch->dr);
    
    //leer cada entrada del directorio /proc/
    while ((arch->di = readdir(arch->dr)) != NULL) {
        // verificamos si el nombre de la entrada es un número (PID)
        if (isNumber(arch->di->d_name)) {
            pid = atoi(arch->di->d_name);
            // Obtener información del proceso
            getProcessCommand(pid, command, sizeof(command));
            memorySize = getProcessMemorySize(pid);
            // Escribir la información al archivo
            fprintf(outputFile, "%-10d\t%-40s\t%ld\n", pid, command, memorySize);
        }
    }
    
    fclose(outputFile);
    printMessage(screen, "Informacion de procesos guardada exitosamente");
}

//funcion para ordenar de forma descendente -> implementada en la func qsort
int compareProcesses(const void *a, const void *b) {
    PROCESS *procA = (PROCESS *)a;
    PROCESS *procB = (PROCESS *)b;
    
    // Ordenar de mayor a menor memoria
    if (procA->memorySize > procB->memorySize) return -1;
    if (procA->memorySize < procB->memorySize) return 1;
    return 0;
}


//funcion que nos guarda en un archivo una liste de n procesos 
void writeTopMemoryProcesses(ARCHIVE *arch, SCREEN *screen, const char *filename, int topN) {
    FILE *outputFile;
    PROCESS *processes;
    int processCount = 0;
    int maxProcesses = 1000; // Límite maximo de procesos
    int pid;  //numero de procesos que se desea imprimir
    
    // Validar parámetros
    if (topN <= 0) {
        printMessage(screen, "Error: El numero debe ser mayor a 0");
        return;
    }
    
    //Asignar memoria para almacenar procesos
    //creamos un arreglo dinamico
    processes = malloc(maxProcesses * sizeof(PROCESS));
    if (processes == NULL) {
        printMessage(screen, "Error: No se pudo asignar memoria");
        return;
    }
    
    //reiniciar el directorio para leer desde el principio
    rewinddir(arch->dr);
    
    //leer todos los procesos y almacenar su información
    while ((arch->di = readdir(arch->dr)) != NULL && processCount < maxProcesses) {
        //verificar si el nombre de la entrada es un número (PID)
        if (isNumber(arch->di->d_name)) {
            pid = atoi(arch->di->d_name);
            
            //obtener información del proceso
            getProcessCommand(pid, processes[processCount].command, sizeof(processes[processCount].command));
            processes[processCount].memorySize = getProcessMemorySize(pid);
            processes[processCount].pid = pid;
            
            //solo contar procesos que usan memoria
            if (processes[processCount].memorySize > 0) {
                processCount++;
            }
        }
    }
    
    //ordenar procesos por uso de memoria (descendente)
    //la funcion qsort se encuentra en la libreria "stdlib.h"
    qsort(processes, processCount, sizeof(PROCESS), compareProcesses);
    
    //abrir archivo para escritura
    outputFile = fopen(filename, "w");
    if (outputFile == NULL) {
        printMessage(screen, "Error: No se pudo crear el archivo");
        free(processes);
        return;
    }
    
    //escribir cabecera
    fprintf(outputFile, "TOP %d PROCESOS CON MAS MEMORIA\n", (topN > processCount) ? processCount : topN);
    fprintf(outputFile, "================================\n");
    fprintf(outputFile, "PID\tCOMMAND\t\tSIZE(kB)\n");
    fprintf(outputFile, "---\t-------\t\t--------\n");
    
    //escribir los top N procesos
    int limit = (topN > processCount) ? processCount : topN;
    for (int i = 0; i < limit; i++) {
        fprintf(outputFile, "%d\t%-15s\t%ld\n", 
                processes[i].pid, 
                processes[i].command, 
                processes[i].memorySize);
    }
    
    fclose(outputFile);
    free(processes);
    
    char message[100];
    snprintf(message, sizeof(message), "Top %d procesos guardados en %s", limit, filename);
    printMessage(screen, message);
}


//FUNCIONES DE IMPRECION-------------------------------------------------------------------

//FUNCION QUE NOS AYUDA A CONTROLAS LA IMPRECION DE LAS VENTANAS 
int windowcontrol(SCREEN *screen,BUFFER *buffer, ARCHIVE *arch,CLI *cC){
    //verifica si el windipdate es 1 para reimprimir el contenido de la ventana
    if(screen->windupdate[0]){
        printbuffer(screen, buffer);
    }
    if(screen->windupdate[1]){
        printMessage(screen, cC->message);
    }
    if(screen->windupdate[2]){
        printArchive(screen, arch,buffer,cC);
    }
    return 0;
}


//funcion para imprimir el buffer
void printbuffer(SCREEN *screen, BUFFER *bufferControl){
    werase(screen->wind[0]);//limpia el contenido de la ventana
    box(screen->wind[0], 0, 0);//dibuja un cuadro en el borde o alrrededor de la ventana
    mvwprintw(screen->wind[0],1,1,"-> ");  
    mvwprintw(screen->wind[0],1,3,"%s",bufferControl->buffer);//cordenadas donde se imprimara el contenido del buffer
    screen->windupdate[0] = 0;//ponemos el estdo en ya actualizado
    wrefresh(screen->wind[0]);//refresca la ventana

}

//funcion para imprimir mensajes
void printMessage(SCREEN *screen, char *message){
    werase(screen->wind[1]);
    box(screen->wind[1], 0, 0);
    mvwprintw(screen->wind[1],1,1,"%s",message);//cordenas en donde imprimimos el mensaje
    screen->windupdate[1] = 0;
    wrefresh(screen->wind[1]);
}

//funcion para imprimir los procesos o el directorio /proc/
void printArchive(SCREEN *screen, ARCHIVE *arch, BUFFER *bff, CLI *cli) {
    FILE *file = fopen(cli->namearch, "r");
    werase(screen->wind[2]);
    box(screen->wind[2], 0, 0);
    
    if (!file) {
        mvwprintw(screen->wind[2], 2, 1, "No se pudo abrir el archivo %s", cli->namearch);
        wrefresh(screen->wind[2]);
        return;
    }
    
    //solo leer el archivo si es diferente al anterior o si no se ha leído
    if (strcmp(cli->currentFile, cli->namearch) != 0) {
        cli->maxrow = 0;
        cli->row = 0;  // Resetear posición al cambiar archivo
        strcpy(cli->currentFile, cli->namearch);
        
        //leer todas las líneas del archivo
        //guardamos cada liena en un arreglo
        while (fgets(cli->lines[cli->maxrow], sizeof(cli->lines[cli->maxrow]), file) && cli->maxrow < 1000) {
            cli->lines[cli->maxrow][strcspn(cli->lines[cli->maxrow], "\n")] = '\0';
            cli->maxrow++;
        }
    }
    fclose(file);
    
    //validar límites del scroll
    int windowHeight = 32; // 34 - 2 (bordes)
    int maxScroll = cli->maxrow - windowHeight;
    if (maxScroll < 0) maxScroll = 0;
    if (cli->row > maxScroll) cli->row = maxScroll;
    if (cli->row < 0) cli->row = 0;
    
    //mostrar información del archivo y posición
    mvwprintw(screen->wind[2], 1, 1, "Archivo: %s (Linea %d-%d de %d)", cli->namearch, cli->row + 1, (cli->row + windowHeight > cli->maxrow) ? cli->maxrow : cli->row + windowHeight, cli->maxrow);
    
    //mostrar las líneas visibles
    int displayLine = 2; //empezar en línea 2 (después del título)
    int linesToShow = (cli->maxrow < windowHeight) ? cli->maxrow : windowHeight;
    
    for (int i = 0; i < linesToShow && (cli->row + i) < cli->maxrow && displayLine < 34; i++) {
        mvwprintw(screen->wind[2], displayLine++, 1, "%s", cli->lines[cli->row + i]);
    }
    box(screen->wind[2], 0, 0);
    screen->windupdate[2] = 0;
    wrefresh(screen->wind[2]);
}
