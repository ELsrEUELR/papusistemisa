#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>

#include "funtop.h"
#include "proyct2.h"
#include "lista.h"

void initializecore(CORE *core){
    core->IDproccesor=0;
    core->cuantum = 3;
    core->cyclecounter = 0;
    core->completedcycles = 1;
    core->maxCycles = 1000000;
    core->LISTEJECUCION = NULL;
    core->LISTESPERA = NULL;

    core->intructionsID = 0;
}

void comandPROY2(SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core) {
    int n;
    NODO *nodo= NULL;
    //dependiendo de el numero de palabras ingresadas se manejaran los comandos
    switch (bff->commandstatus) {
        case 1: //Solo comando

            if (strcmp(bff->command, "exit") == 0){ //verificamos si escribimos "exit"
                cli->cliState = -1; //en caso de si, poner el estado de la cli para salir
            }
            else if(strcmp(bff->command, "exit..")==0){
                cli->cliState = 1;
                sc->windupdate[2] = 1;
            }
            else if(strcmp(bff->command, "LOAD") == 0){
                strcpy(cli->message1, "load 'nombre del archivo'");
                printMessage(sc,cli);
            }
        break;
        case 2:
            if(strcmp(bff->command, "LOAD") == 0){
                FILE *arch;
                if(bff->parameter1[0] != '\0'){
                    arch = fopen(bff->parameter1, "r");
                    if(arch != NULL){ 
                        nodo = crear_nodo(core->IDproccesor, bff->parameter1);
                        
                        if(nodo != NULL){  
                            core->IDproccesor++;
                            insertar_nodo(&core->LISTESPERA, nodo);
                            
                            strcpy(cli->message2, "Proceso cargado exitosamente");
                            printMessage(sc, cli);
                        }
                        sc->windupdate[3] = 1;
                        
                        fclose(arch);
                    } else {
                        strcpy(cli->message1, "ERROR: No se pudo abrir el archivo");
                        printMessage(sc, cli);
                    }
                }
            }
        break;
        case 3: // Comando + 2 parámetros
        break;

        default:
        break;
    }
}

void frecuency_execution(SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core){
    core->cyclecounter++;

    if(core->cyclecounter > core->maxCycles){
        core->cyclecounter = 0;
        core->completedcycles++;
        sc->windupdate[3] = 1;
        sc->windupdate[4] = 1;
    }

    if(core->completedcycles > core->cuantum){
        core->completedcycles = 1;
    }
}

void execution_processes(SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core){
    if(core->cyclecounter == core->maxCycles){
        if(core->LISTEJECUCION != NULL){
            core->LISTEJECUCION->PC++;
            char *linea = leer_renglon(core->LISTEJECUCION->name, core->LISTEJECUCION->PC);
            if(linea != NULL){
                strncpy(core->LISTEJECUCION->IR, linea, 31);  // ✅ Copiar máximo 31 chars
                core->LISTEJECUCION->IR[31] = '\0';           // ✅ Asegurar terminación
                free(linea);
            } else {
                strcpy(core->LISTEJECUCION->IR, "end");
            }
            BUFFER command;
            InitializeBuffer(&command);
            load_instructions(sc,&command,cli,core);
            executeIsttruccion(sc,&command,core, cli);
            sc->windupdate[4] = 1;
        }
    }
}

void verifyexecution(SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core){
    NODO *nodo = NULL;
    if(core->completedcycles == core->cuantum){
        if(core->LISTEJECUCION == NULL){
            if(core->LISTESPERA != NULL){
                nodo = extraer(&core->LISTESPERA);
                insertar_nodo(&core->LISTEJECUCION, nodo);
            }
        }
        else{
            if(core->LISTESPERA != NULL){
                nodo = extraer(&core->LISTEJECUCION);
                insertar_nodo(&core->LISTESPERA, nodo);
                nodo = extraer(&core->LISTESPERA);
                insertar_nodo(&core->LISTEJECUCION, nodo);
            }
        }
    }

    if(core->cyclecounter == core->maxCycles){
        execution_processes(sc,bff,cli,core);
    }
    
}

int load_instructions (SCREEN *sc, BUFFER *comand_buffer, CLI *cli,CORE *core){
    NODO *nodo = core->LISTEJECUCION;
    char mensaje[200];
    mensaje[0] = '\0';
    if(core->LISTEJECUCION == NULL){
        return -1;
    }
    stringToUpper(nodo->IR);
    sscanf(nodo->IR,"%s %s %s %s",comand_buffer->command,comand_buffer->parameter1,comand_buffer->parameter2,comand_buffer->remaining);
    if (!strcmp(comand_buffer->command, "MOV")){
        if (comand_buffer->parameter1[0] == 0 || comand_buffer->parameter2[0] == 0){
            sprintf(mensaje, "Syntax Error-> proceso [%d] : PC[%d] ", nodo->id, nodo->PC);
            strcpy(cli->message1, mensaje);
            strcpy(cli->message2,"parametro faltante para MOV");
            printMessage(sc, cli);
            return -1;
        }
        core->intructionsID = 1;
        return 0;
    }
    if (!strcmp(comand_buffer->command, "ADD")){
        if (comand_buffer->parameter1[0] == 0 || comand_buffer->parameter2[0] == 0){
            sprintf(mensaje, "Syntax Error-> proceso [%d] : PC[%d] ", nodo->id, nodo->PC);
            strcpy(cli->message1, mensaje);
            strcpy(cli->message2,"parametro faltante para ADD");
            printMessage(sc, cli);
            return -1;
        }
        core->intructionsID = 2;
        return 0;
    }
    if (!strcmp(comand_buffer->command, "SUB")){
        if (comand_buffer->parameter1[0] == 0 || comand_buffer->parameter2[0] == 0){
            sprintf(mensaje, "Syntax Error-> proceso [%d] : PC[%d] ", nodo->id, nodo->PC);
            strcpy(cli->message1, mensaje);
            strcpy(cli->message2,"parametro faltante para SUB");
            printMessage(sc, cli);
            return -1;
        }
        core->intructionsID = 3;
        return 0;
    }
    if (!strcmp(comand_buffer->command, "MUL")){
        if (comand_buffer->parameter1[0] == 0 || comand_buffer->parameter2[0] == 0){
            sprintf(mensaje, "Syntax Error-> proceso [%d] : PC[%d] ", nodo->id, nodo->PC);
            strcpy(cli->message1, mensaje);
            strcpy(cli->message2,"parametro faltante para MUL");
            printMessage(sc, cli);
            return -1;
        }
        core->intructionsID = 4;
        return 0;
    }
    if (!strcmp(comand_buffer->command, "DIV")){
        if (comand_buffer->parameter1[0] == 0 || comand_buffer->parameter2[0] == 0){
            sprintf(mensaje, "Syntax Error-> proceso [%d] : PC[%d] ", nodo->id, nodo->PC);
            strcpy(cli->message1, mensaje);
            strcpy(cli->message2,"parametro faltante para DIV");
            printMessage(sc, cli);
            return -1;
        }
        core->intructionsID = 5;
        return 0;
    }
    if (!strcmp(comand_buffer->command, "INC")){
        if (comand_buffer->parameter1[0] == 0){
            sprintf(mensaje, "Syntax Error-> proceso [%d] : PC[%d] ", nodo->id, nodo->PC);
            strcpy(cli->message1, mensaje);
            strcpy(cli->message2,"parametro faltante para INC");
            printMessage(sc, cli);
            return -1;
        }
        core->intructionsID = 6;
        return 0;
    }
    if (!strcmp(comand_buffer->command, "DEC")){
        if (comand_buffer->parameter1[0] == 0){
            sprintf(mensaje, "Syntax Error-> proceso [%d] : PC[%d] ", nodo->id, nodo->PC);
            strcpy(cli->message1, mensaje);
            strcpy(cli->message2,"parametro faltante para DEC");
            printMessage(sc, cli);
            return -1;
        }
        core->intructionsID = 7;
        return 0;
    }
    if(!strcmp(comand_buffer->command, "END")){
        printLISTexecute(sc,cli,core);
        sprintf(mensaje,"SE LEEYO O SE ENCONTRO EL FIN DEL ARCHIVO-> proceso[%d]  PC[%d]", nodo->id, nodo->PC);
        strcpy(cli->message1,"");
        strcpy(cli->message2,mensaje);
        printMessage(sc,cli);
        nodo = extraer(&core->LISTEJECUCION);
        eliminar_nodo(&nodo);
        return 0;
        core->intructionsID = 8;
    }
    else{
        core->intructionsID == 0;
    }
}

int executeIsttruccion(SCREEN *sc,BUFFER *comand,CORE * core, CLI *cli){
    char mensaje[200];
    mensaje[0] = '\0';
    
    if(core->LISTEJECUCION == NULL){
        strcpy(cli->message1, "ERROR: No hay proceso en ejecución");
        printMessage(sc, cli);
        return -1;
    }

    int par1 = stringToRegisterIndex(comand->parameter1);
    int par2 = 0;
    int numero = 0;
    if(core->intructionsID == 0){
        sprintf(mensaje, "->ERROR DE INSTRUCCION-> proceso [%d] : PC[%d] ", core->LISTEJECUCION->id, core->LISTEJECUCION->PC);
        strcpy(cli->message1, mensaje);
        strcpy(cli->message2,"NO SE RECONOCE INSTRUCCION: [MOV][SUB][ADD][MUL]...");
        printMessage(sc, cli);
        return -1;
    }
    if(par1 < 0){
        sprintf(mensaje, "-->ERROR DE PARAMETRO1-> proceso [%d] : PC[%d] ", core->LISTEJECUCION->id, core->LISTEJECUCION->PC);
        strcpy(cli->message1, mensaje);
        strcpy(cli->message2,"REGISTRO INCORRECTO: [AX][BX][CX][DX]");
        printMessage(sc, cli);
        return -1;
    }
    if(comand->parameter2[0] != 0){
        if(isNumber(comand->parameter2)){
            par2 = stringToInteger(comand->parameter2);
        }
        else{
            par2 = stringToRegisterIndex(comand->parameter2);
            if(par2 < 0){
                sprintf(mensaje, "-->ERROR DE PARAMETRO 2-> proceso [%d] : PC[%d] ", core->LISTEJECUCION->id, core->LISTEJECUCION->PC);
                strcpy(cli->message1, mensaje);
                strcpy(cli->message2,"PARAMETRO INCORRECTO: [AX][BX][CX][DX] - 1,2,3...,N");
                printMessage(sc, cli);
                return -1; 
            }
            numero = 1;
        }
    }

    switch(core->intructionsID){
        case 1:
            instructionMOV(core->LISTEJECUCION, par1, par2, numero);
        break;
        case 2:
            instructionADD(core->LISTEJECUCION, par1, par2, numero);
        break;
        case 3:
            instructionSUB(core->LISTEJECUCION, par1, par2, numero);
        break;
        case 4:
            instructionMUL(core->LISTEJECUCION, par1, par2, numero);
        break;
        case 5:
            if (!numero && !par2){
                strcpy(cli->message1, "Error de division");
                strcpy(cli->message2,"no se puede dividir entre 0");
                printMessage(sc,cli);
                return -1;
            }
            if (numero && core->LISTEJECUCION->Xregisters[par2] == 0){
                strcpy(cli->message1, "Error de division");
                strcpy(cli->message2,"no se puede dividir entre 0");
                printMessage(sc,cli);
                return -1;
            }
            instructionDIV(core->LISTEJECUCION, par1, par2, numero);
        break;
        case 6:
            instructionINC(core->LISTEJECUCION, par1, par2, numero);
        break;
        case 7:
            instructionDEC(core->LISTEJECUCION, par1, par2, numero);
        break;
        default:
        break;

    }
}

//==================================

int instructionMOV(NODO* pcb, int p1, int p2, int mode){
    if (mode) pcb->Xregisters[p1] = pcb->Xregisters[p2];
    else pcb->Xregisters[p1] = p2;
    return 0;
}

int instructionADD(NODO* pcb, int p1, int p2, int mode){
    if (mode) pcb->Xregisters[p1] += pcb->Xregisters[p2];
    else pcb->Xregisters[p1] += p2;
    return 0;
}

int instructionSUB(NODO* pcb, int p1, int p2, int mode){
    if (mode) pcb->Xregisters[p1] -= pcb->Xregisters[p2];
    else pcb->Xregisters[p1] -= p2;
    return 0;
}

int instructionDIV(NODO* pcb, int p1, int p2, int mode){
    if (mode) pcb->Xregisters[p1] /= pcb->Xregisters[p2];
    else pcb->Xregisters[p1] /= p2;
    return 0;
}

int instructionMUL(NODO* pcb, int p1, int p2, int mode){
    if (mode) pcb->Xregisters[p1] *= pcb->Xregisters[p2];
    else pcb->Xregisters[p1] *= p2;
    return 0;
}

int instructionINC(NODO* pcb, int p1, int p2, int mode){
    pcb->Xregisters[p1]++;
    return 0;
}

int instructionDEC(NODO* pcb, int p1, int p2, int mode){
    pcb->Xregisters[p1]--;
    return 0;
}

//==================================
//FUNCIONES AUXILIARES======================================================

char *leer_renglon(const char *archivoNombre, int n) {
    FILE *archivo = fopen(archivoNombre, "r");
    if (!archivo) return NULL;
    
    char *linea = NULL;
    size_t len = 0;
    ssize_t read;
    int contador = 0;
    
    while ((read = getline(&linea, &len, archivo)) != -1) {
        // ✅ Eliminar saltos de línea y espacios al final
        while (read > 0 && (linea[read-1] == '\n' ||
                            linea[read-1] == '\r' ||
                            linea[read-1] == ' ' ||
                            linea[read-1] == '\t')) {
            linea[--read] = '\0';
        }
        
        // ✅ Ignorar líneas vacías o solo con espacios
        if (read == 0 || linea[0] == '\0') {
            continue; // NO liberar aquí, getline reutiliza el buffer
        }
        
        // Contar solo líneas válidas
        contador++;
        if (contador == n) {
            fclose(archivo);
            return linea; // ✅ Retorna la línea SIN salto de línea
        }
    }
    
    // ✅ Liberar memoria si no se encontró la línea
    fclose(archivo);
    free(linea);
    return NULL;
}

int stringToUpper(char* str){
    for (int i = 0; str[i] != 0; i++)
        str[i] = toupper(str[i]);
    return 0;
}

int stringToInteger(char* str){
    char* rem;
    int value = strtol(str, &rem, 10);
    return value;
}

int stringToRegisterIndex(char* str){
    stringToUpper(str);
    if (!strcmp(str, "AX")) return 0;
    if (!strcmp(str, "BX")) return 1;
    if (!strcmp(str, "CX")) return 2;
    if (!strcmp(str, "DX")) return 3;
    return -1;
}

//IMPRECION DE LAS LISTAS

void printLISTwaiting(SCREEN * SC, CLI *cli,CORE *c){
    NODO *actual = c->LISTESPERA;
    int i = 1;
    
    werase(SC->wind[3]);
    box(SC->wind[3], 0, 0);
    mvwprintw(SC->wind[3],1,1,"LISTA DE ESPERA");
    
    if(cli->cliState == 3){
        if(actual == NULL){
            mvwprintw(SC->wind[3],2,1,"->>SIN PROCESOS");
        }
        else {
            while(actual != NULL){  
                mvwprintw(SC->wind[3], i * 2, 1,"o>>>>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d] ", actual->id, actual->name, actual->Xregisters[0], actual->Xregisters[1], actual->Xregisters[2], actual->Xregisters[3]); 
                
                // ✅ Mostrar IR solo si no está vacío o no es END
                if(actual->IR[0] != '\0' && strcmp(actual->IR, "END") != 0){
                    mvwprintw(SC->wind[3], (i * 2)+1, 1,"                            PC[%4d] IR[%s]",
                             actual->PC, actual->IR);
                } else {
                    mvwprintw(SC->wind[3], (i * 2)+1, 1,"                            PC[%4d] IR[-]",
                             actual->PC);
                }
                
                actual = actual->sig;
                i++;
                if(i >= 32) break; 
            }
        }
    }
    else{
        mvwprintw(SC->wind[3],2,1,"not found in mode");
    }
    SC->windupdate[3] = 0;
    wrefresh(SC->wind[3]);
}


void printLISTexecute(SCREEN * SC, CLI *cli,CORE *c){
    NODO *actual = c->LISTEJECUCION;
    int i = 1;
    
    werase(SC->wind[4]);  // ✅ Cambiar a wclear
    box(SC->wind[4], 0, 0);
    mvwprintw(SC->wind[4],1,1,"LISTA DE EJECUCION");
    
    if(cli->cliState == 3){
        if(actual == NULL){
            mvwprintw(SC->wind[4],2,1,"-->> ");
        }
        else {
            while(actual != NULL){  
                mvwprintw(SC->wind[4], i * 2, 1,"-->>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d] ", actual->id, actual->name, actual->Xregisters[0], actual->Xregisters[1], actual->Xregisters[2], actual->Xregisters[3]); 
                mvwprintw(SC->wind[4], (i * 2)+1, 1,"                           PC[%4d] IR[%s]",actual->PC, actual->IR);
                actual = actual->sig;
                i++;
                if(i >= 32) break; 
            }
        }
    }
    else{
        mvwprintw(SC->wind[4],1,1,"not found in mode");
    }
    SC->windupdate[4] = 0;
    wrefresh(SC->wind[4]);
}