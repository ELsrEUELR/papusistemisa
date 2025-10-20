#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>

#include "funtop.h"
#include "proyct2.h"
#include "lista.h" 

void initializecore(CORE *core){
    core->IDproccesor=0;
    core->cuantum = 5;
    core->cyclecounter = 0;
    core->completedcycles = 1;
    core->maxCycles = 1000000;
    core->LISTEJECUCION[0] = NULL;
    core->LISTEJECUCION[1] = NULL;
    core->LISTEJECUCION[2] = NULL;
    core->LISTEJECUCION[3] = NULL;
    core->LISTESPERA = NULL;

    core->intructionsID = 0;

    pthread_mutex_init(&core->mutex, NULL);
}

void comandPROY2(SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core) {
    int n;
    NODO *nodo= NULL;
    //dependiendo de el numero_reg de palabras ingresadas se manejaran los comandos
    switch (bff->commandstatus) {
        case 1: //Solo comando

            if (strcmp(bff->command, "exit") == 0){ //verificamos si escribimos "exit"
                cli->cliState = -1; //en caso de si, poner el estado de la cli para salir
            }
            else if(strcmp(bff->command, "exit..")==0){//salir al estado principal de la cli
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

void frecuency_execution(SCREEN *sc, BUFFER *bff, CLI *cli, CORE *core){///MANEJADOR DE LA FRECUANCIA
    core->cyclecounter++;

    if(core->cyclecounter >= core->maxCycles){//verificar el millon de ciclos completado
        core->cyclecounter = 0;
        core->completedcycles++;                //se completo un cliclo
        
        char msg[200];
        sprintf(msg, "Ciclo completado: %d/%d del quantum", core->completedcycles, core->cuantum);
        
        strcpy(cli->message1, msg);
        //actualizanmos ventanas
        sc->windupdate[1] = 1;
        sc->windupdate[3] = 1;
        sc->windupdate[4] = 1;
    }

    //verifica que ya se llego el cuantum = 5
    if(core->completedcycles >= core->cuantum){
        strcpy(cli->message1, ">> QUANTUM COMPLETADO ");
        printMessage(sc, cli);
    }
}

void execution_processes(SCREEN *sc, BUFFER *bff, CLI *cli, CORE *core, int n_lista) {
    if(core->cyclecounter == 0) {

        if(core->LISTEJECUCION[n_lista] != NULL) {
            core->LISTEJECUCION[n_lista]->PC++;

            char *linea = leer_renglon(core->LISTEJECUCION[n_lista]->name, core->LISTEJECUCION[n_lista]->PC);
            
            if(core->LISTEJECUCION[n_lista] != NULL){
                if(linea != NULL) {
                    strncpy(core->LISTEJECUCION[n_lista]->IR, linea, 31);//COPEAMOS LA CADENA QUE SE GUARDO EN LINEA A IR DE LA LISTA DE EJECUACION NUMERO N;
                    core->LISTEJECUCION[n_lista]->IR[31] = '\0';
                    free(linea);
                } else {
                    strcpy(core->LISTEJECUCION[n_lista]->IR, "end");
                }
                //DECLARAMOS VARIABLE TIPO BUFFER
                BUFFER command;
                InitializeBuffer(&command); //LA INICIALIZAMOS
                
                int result = load_instructions(sc, &command, cli, core, n_lista); //REGRESA 0 SI LA INTRUCCION ESTA CORRECTA O -1 SI ESTA MAL
                
                if(result == 0) {  
                    executeIsttruccion(sc, &command, core, cli, n_lista);//EJECUTA LA INTRUCCION
                }
            }else{
            }
        }else {
        }

    }

}

void verifyexecution(SCREEN *sc, BUFFER *bff, CLI *cli, CORE *core) {//VERIFICAMOS Y HACEMOS EL MOVIMENTO DE NODOS ENTRE LISTAS
    NODO *nodo = NULL;
    int n_nodos = 0;
    int num_hilos_creados = 0;
    char msg[200];


    if(core->completedcycles >= core->cuantum){
        
        // ========== CASO 1: Listas de ejecución vacías ==========
        int todas_vacias = 1;
        for(int j = 0; j < 4; j++) {
            if(core->LISTEJECUCION[j] != NULL) {
                todas_vacias = 0;
                break;
            }
        }
        
        if(todas_vacias) {
            if(core->LISTESPERA != NULL) {
                n_nodos = contarNODO(core->LISTESPERA);
        
                sprintf(msg, "1>> Inicializando: %d procesos en espera", n_nodos);
                strcpy(cli->message4, msg);
                printMessage(sc, cli);
                
                if(n_nodos == 1) {
                    nodo = extraer(&core->LISTESPERA);
                    
                    if(nodo != NULL) {
                        insertar_nodo(&core->LISTEJECUCION[0], nodo);
                    }
                    
                    strcpy(cli->message3, ">> Modo: 1 proceso (sin hilos)");
                    printMessage(sc, cli);
                }
                else if(n_nodos >= 2) {
                    // Crear hilos basándose en el conteo
                    int hilos_a_crear = (n_nodos > 4) ? 4 : n_nodos;
                    
                    // Informar creación de hilos
                    sprintf(msg, ">> 1Creando %d HILOS para mover procesos...", hilos_a_crear);
                    strcpy(cli->message3, msg);
                    printMessage(sc, cli);
                    
                    for(int i = 0; i < hilos_a_crear; i++) {
                        THR_ARG *args = malloc(sizeof(THR_ARG));
                        if(args == NULL) break;
                        
                        args->LISTAEJEC = &core->LISTEJECUCION[i];
                        args->LISTAESPE = &core->LISTESPERA;
                        args->numero_nodo = i;
                        args->mutex = &core->mutex;
                        
                        pthread_create(&core->thread_FOR_list[i], NULL, thread_move_nodo, args);
                        num_hilos_creados++;
                    }
                    
                    // Esperar a que todos los hilos terminen
                    for(int j = 0; j < num_hilos_creados; j++) {
                        pthread_join(core->thread_FOR_list[j], NULL);
                    }
                    
                    //  Confirmar finalización
                    sprintf(msg, "1>> %d hilos finalizados - Procesos movidos a ejecucion", num_hilos_creados);
                    strcpy(cli->message3, msg);
                    printMessage(sc, cli);

                    sc->windupdate[3] = 1;
                    sc->windupdate[4] = 1;
                }
            }
        }
        // ========== CASO 2: Ya hay procesos en ejecución ==========
        else {
            if(core->LISTESPERA != NULL) {
                // Cambio de contexto
                strcpy(cli->message2, "2>> CAMBIO DE CONTEXTO: Moviendo procesos...");
                printMessage(sc, cli);
                
                // Mover procesos actuales de vuelta a espera
                for(int j = 0; j < 4; j++) {
                    if(core->LISTEJECUCION[j] != NULL) {
                        nodo = extraer(&core->LISTEJECUCION[j]);
                        insertar_nodo(&core->LISTESPERA, nodo);
                    }
                }
                
                n_nodos = contarNODO(core->LISTESPERA);
                sprintf(msg, "2>> Inicializando: %d procesos en espera", n_nodos);
                strcpy(cli->message4, msg);
                
                if(n_nodos == 1) {
                    nodo = extraer(&core->LISTESPERA);
                    
                    if(nodo != NULL) {
                        insertar_nodo(&core->LISTEJECUCION[0], nodo);
                    }
                }
                else if(n_nodos >= 2) {
                    int hilos_a_crear = (n_nodos > 4) ? 4 : n_nodos;
                    num_hilos_creados = 0;
                    
                    sprintf(msg, "2>> Cambio contexto con %d HILOS", hilos_a_crear);
                    strcpy(cli->message3, msg);
                    printMessage(sc, cli);
                    
                    for(int i = 0; i < hilos_a_crear; i++) {

                        THR_ARG *args = malloc(sizeof(THR_ARG));
                        if(args == NULL) break;
                        
                        args->LISTAEJEC = &core->LISTEJECUCION[i];
                        args->LISTAESPE = &core->LISTESPERA;
                        args->numero_nodo = i;
                        args->mutex = &core->mutex;
                        
                        pthread_create(&core->thread_FOR_list[i], NULL, thread_move_nodo, args);
                        num_hilos_creados++;
                    }
                    
                    // Esperar a que todos los hilos terminen
                    for(int j = 0; j < hilos_a_crear; j++) {
                        pthread_join(core->thread_FOR_list[j], NULL);
                    }

                    sc->windupdate[3] = 1;
                    sc->windupdate[4] = 1;
                }
            }
        }
        //REINICIAMOS EL CONTADOR DE INSTRUCCIONES
        core->completedcycles = 0;  
        strcpy(cli->message4, ">> Quantum reseteado");
        printMessage(sc, cli);
    }
    
    // ========== Ejecutar procesos ==========
    if(core->cyclecounter == 0) {  //SI EL CONTADOR DE CICLOS ES 0 SIGNIFICA QUE SE DEBE EJECUTAR UNA NUEVA INTRUCCION DEL PROCESO
        execute_all_processes_threaded(sc, bff, cli, core);//FUNCION PARA EJECUTAR LOS PROCESOS EN HILOS
    }
    
}

int load_instructions (SCREEN *sc, BUFFER *comand_buffer, CLI *cli, CORE *core, int n_lista){
    char mensaje[200];
    mensaje[0] = '\0';
    
    
    if(core->LISTEJECUCION[n_lista] == NULL){
        return -1;
    }
    
    NODO *nodo = core->LISTEJECUCION[n_lista];
    
    char ir_copy[32];
    strncpy(ir_copy, nodo->IR, 31);
    ir_copy[31] = '\0';
    
    stringToUpper(ir_copy);
    sscanf(ir_copy, "%s %s %s %s", comand_buffer->command, comand_buffer->parameter1, comand_buffer->parameter2, comand_buffer->remaining);
    
    //  Verificar MOV
    if (!strcmp(comand_buffer->command, "MOV")){
        if (comand_buffer->parameter1[0] == 0 || comand_buffer->parameter2[0] == 0){
            return -1;
        }
        core->intructionsID = 1;
        return 0;
    }
    
    //  Verificar ADD
    if (!strcmp(comand_buffer->command, "ADD")){
        if (comand_buffer->parameter1[0] == 0 || comand_buffer->parameter2[0] == 0){
            return -1;
        }
        core->intructionsID = 2;
        return 0;
    }
    
    //  Verificar SUB
    if (!strcmp(comand_buffer->command, "SUB")){
        if (comand_buffer->parameter1[0] == 0 || comand_buffer->parameter2[0] == 0){
            return -1;
        }
        core->intructionsID = 3;
        return 0;
    }
    
    //  Verificar MUL
    if (!strcmp(comand_buffer->command, "MUL")){
        if (comand_buffer->parameter1[0] == 0 || comand_buffer->parameter2[0] == 0){
            return -1;
        }
        core->intructionsID = 4;
        return 0;
    }
    
    //  Verificar DIV
    if (!strcmp(comand_buffer->command, "DIV")){
        if (comand_buffer->parameter1[0] == 0 || comand_buffer->parameter2[0] == 0){
            return -1;
        }
        core->intructionsID = 5;
        return 0;
    }
    
    //  Verificar INC
    if (!strcmp(comand_buffer->command, "INC")){
        if (comand_buffer->parameter1[0] == 0){
            return -1;
        }
        core->intructionsID = 6;
        return 0;
    }
    
    //  Verificar DEC
    if (!strcmp(comand_buffer->command, "DEC")){
        if (comand_buffer->parameter1[0] == 0){
            return -1;
        }
        core->intructionsID = 7;
        return 0;
    }
    
    //  Verificar END
    if(!strcmp(comand_buffer->command, "END")){
        NODO *temp = extraer(&core->LISTEJECUCION[n_lista]);
        
        if(temp != NULL){
            eliminar_nodo(&temp);
        }
        
        core->intructionsID = 8;
        return 0;
    }
    
    //  Comando no reconocido
    core->intructionsID = 0;
    return 0;
}


int executeIsttruccion(SCREEN *sc, BUFFER *comand, CORE *core, CLI *cli, int n_lista){
    char mensaje[200];
    mensaje[0] = '\0';
    
    if(core->LISTEJECUCION[n_lista] == NULL){
        return -1;
    }

    int par1 = stringToRegisterIndex(comand->parameter1);
    int par2 = 0;
    int numero_reg = 0;
    
    //  Verificar ID de instrucción
    if(core->intructionsID == 0){
        return -1;
    }
    
    //  Verificar parámetro 1
    if(par1 < 0){
        return -1;
    }
    
    //  Verificar y procesar parámetro 2
    if(comand->parameter2[0] != 0){
        if(isNumber(comand->parameter2)){
            par2 = stringToInteger(comand->parameter2);
        }
        else{
            par2 = stringToRegisterIndex(comand->parameter2);
            if(par2 < 0){
                return -1; 
            }
            numero_reg = 1;
        }
    }

    
    if(core->LISTEJECUCION[n_lista] == NULL){
        return -1;
    }
    
    switch(core->intructionsID){
        case 1:
            instructionMOV(core->LISTEJECUCION[n_lista], par1, par2, numero_reg);
            break;
            
        case 2:
            instructionADD(core->LISTEJECUCION[n_lista], par1, par2, numero_reg);
            break;
            
        case 3:
            instructionSUB(core->LISTEJECUCION[n_lista], par1, par2, numero_reg);
            break;
            
        case 4:
            instructionMUL(core->LISTEJECUCION[n_lista], par1, par2, numero_reg);
            break;
            
        case 5:
            //  Verificar división por cero
            if (!numero_reg && !par2){
                printMessage(sc, cli);
                return -1;
            }
            if (numero_reg && core->LISTEJECUCION[n_lista]->Xregisters[par2] == 0){
                printMessage(sc, cli);
                return -1;
            }
            instructionDIV(core->LISTEJECUCION[n_lista], par1, par2, numero_reg);
            break;
            
        case 6:
            instructionINC(core->LISTEJECUCION[n_lista], par1, par2, numero_reg);
            break;
            
        case 7:
            instructionDEC(core->LISTEJECUCION[n_lista], par1, par2, numero_reg);
            break;
            
        default:
            printMessage(sc, cli);
            return -1;
    }
    
    return 0;
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

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//funciones para los hilos

void execute_all_processes_threaded(SCREEN *sc, BUFFER *bff, CLI *cli, CORE *core) {
    int num_hilos = 0;
    
    for(int i = 0; i < 4; i++) {
        if(core->LISTEJECUCION[i] != NULL) {
            THR_ARG *args = malloc(sizeof(THR_ARG));
            args->sc = sc;
            args->bff = bff;
            args->cli = cli;
            args->core = core;
            args->n_lista = i;
            args->mutex = &core->mutex;
            
            pthread_create(&core->thread_FOR_instructions[i], NULL, thread_instructions, args);
            num_hilos++;
        }
    }
    
    for(int i = 0; i < 4; i++) {
        if(core->LISTEJECUCION[i] != NULL) {
            pthread_join(core->thread_FOR_instructions[i], NULL);
        }
    }

    sc->windupdate[3] = 1;
    sc->windupdate[4] = 1;
}



void *thread_move_nodo(void *argumentos){
    THR_ARG *args = (THR_ARG*)argumentos;
    NODO *nodo = NULL;
    
    pthread_mutex_lock(args->mutex);

    if(*(args->LISTAESPE) != NULL){  
        nodo = extraer(args->LISTAESPE); 
        if(nodo != NULL){
            insertar_nodo(args->LISTAEJEC, nodo); 
        }
    }
    
    pthread_mutex_unlock(args->mutex);
    
    free(args);
    return NULL;
}

void *thread_instructions(void *argumentos){
    THR_ARG *args = (THR_ARG*)argumentos;
    

    pthread_mutex_lock(args->mutex);
    execution_processes(args->sc, args->bff, args->cli, args->core, args->n_lista); 
    pthread_mutex_unlock(args->mutex);

    free(args);
    return NULL;
}

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

//FUNCIONES AUXILIARES======================================================

char *leer_renglon(const char *archivoNombre, int n) {
    FILE *archivo = fopen(archivoNombre, "r");
    if (!archivo) return NULL;
    
    char *linea = NULL;
    size_t len = 0;
    ssize_t read;
    int contador = 0;
    
    while ((read = getline(&linea, &len, archivo)) != -1) {
        //  Eliminar saltos de línea y espacios al final
        while (read > 0 && (linea[read-1] == '\n' ||
                            linea[read-1] == '\r' ||
                            linea[read-1] == ' ' ||
                            linea[read-1] == '\t')) {
            linea[--read] = '\0';
        }
        
        //  Ignorar líneas vacías o solo con espacios
        if (read == 0 || linea[0] == '\0') {
            continue; // NO liberar aquí, getline reutiliza el buffer
        }
        
        // Contar solo líneas válidas
        contador++;
        if (contador == n) {
            fclose(archivo);
            return linea; //  Retorna la línea SIN salto de línea
        }
    }
    
    //  Liberar memoria si no se encontró la línea
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
                
                //  Mostrar IR solo si no está vacío o no es END
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


void printLISTexecute(SCREEN *SC, CLI *cli, CORE *c) {
    werase(SC->wind[4]);
    box(SC->wind[4], 0, 0);
    mvwprintw(SC->wind[4], 1, 1, "LISTA DE EJECUCION");
    
    if(cli->cliState == 3) {
        NODO *listas[4] = {
            c->LISTEJECUCION[0],
            c->LISTEJECUCION[1],
            c->LISTEJECUCION[2],
            c->LISTEJECUCION[3]
        };
        
        int num_procesos = 0;
        for(int i = 0; i < 4; i++) {
            if(listas[i] != NULL) num_procesos++;
        }
        
        if(num_procesos == 0) {
            mvwprintw(SC->wind[4], 2, 1, "-->> SIN PROCESOS");
        }
        else if(num_procesos == 1) {
            //  Un solo proceso
            int fila = 2;
            if(listas[0] != NULL) {
                mvwprintw(SC->wind[4], fila, 1, 
                    "-->>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d]",
                    listas[0]->id, listas[0]->name, 
                    listas[0]->Xregisters[0], listas[0]->Xregisters[1],
                    listas[0]->Xregisters[2], listas[0]->Xregisters[3]);
                mvwprintw(SC->wind[4], fila + 1, 1,
                    "                           PC[%4d] IR[%s]",
                    listas[0]->PC, listas[0]->IR);
            }
        }
        else if(num_procesos == 2) {
            //  Dos procesos (lado a lado)
            int fila = 2;
            int idx1 = -1, idx2 = -1;
            
            // Encontrar los dos procesos
            for(int i = 0; i < 4; i++) {
                if(listas[i] != NULL) {
                    if(idx1 == -1) idx1 = i;
                    else if(idx2 == -1) idx2 = i;
                }
            }
            
            if(idx1 != -1 && idx2 != -1) {
                mvwprintw(SC->wind[4], fila, 1,
                    "-->>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d]       "
                    "-->>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d]",
                    listas[idx1]->id, listas[idx1]->name,
                    listas[idx1]->Xregisters[0], listas[idx1]->Xregisters[1],
                    listas[idx1]->Xregisters[2], listas[idx1]->Xregisters[3],
                    listas[idx2]->id, listas[idx2]->name,
                    listas[idx2]->Xregisters[0], listas[idx2]->Xregisters[1],
                    listas[idx2]->Xregisters[2], listas[idx2]->Xregisters[3]);
                    
                mvwprintw(SC->wind[4], fila + 1, 1,
                    "                           PC[%4d] IR[%-20s]     "
                    "                  PC[%4d] IR[%s]",
                    listas[idx1]->PC, listas[idx1]->IR,
                    listas[idx2]->PC, listas[idx2]->IR);
            }
        }
        else if(num_procesos == 3) {
            //  Tres procesos (2 arriba, 1 abajo)
            int fila = 2;
            int indices[3];
            int count = 0;
            
            for(int i = 0; i < 4 && count < 3; i++) {
                if(listas[i] != NULL) {
                    indices[count++] = i;
                }
            }
            
            // Primeros dos lado a lado
            mvwprintw(SC->wind[4], fila, 1,
                "-->>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d]       "
                "-->>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d]",
                listas[indices[0]]->id, listas[indices[0]]->name,
                listas[indices[0]]->Xregisters[0], listas[indices[0]]->Xregisters[1],
                listas[indices[0]]->Xregisters[2], listas[indices[0]]->Xregisters[3],
                listas[indices[1]]->id, listas[indices[1]]->name,
                listas[indices[1]]->Xregisters[0], listas[indices[1]]->Xregisters[1],
                listas[indices[1]]->Xregisters[2], listas[indices[1]]->Xregisters[3]);
                
            mvwprintw(SC->wind[4], fila + 1, 1,
                "                           PC[%4d] IR[%-20s]     "
                "                  PC[%4d] IR[%s]",
                listas[indices[0]]->PC, listas[indices[0]]->IR,
                listas[indices[1]]->PC, listas[indices[1]]->IR);
            
            // Tercero abajo
            fila += 3;  //  IMPORTANTE: Incrementar fila
            mvwprintw(SC->wind[4], fila, 1,
                "-->>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d]",
                listas[indices[2]]->id, listas[indices[2]]->name,
                listas[indices[2]]->Xregisters[0], listas[indices[2]]->Xregisters[1],
                listas[indices[2]]->Xregisters[2], listas[indices[2]]->Xregisters[3]);
                
            mvwprintw(SC->wind[4], fila + 1, 1,
                "                           PC[%4d] IR[%s]",
                listas[indices[2]]->PC, listas[indices[2]]->IR);
        }
        else if(num_procesos == 4) {
            //  Cuatro procesos (2 arriba, 2 abajo)
            int fila = 2;
            
            // Primeros dos
            mvwprintw(SC->wind[4], fila, 1,
                "-->>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d]       "
                "-->>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d]",
                listas[0]->id, listas[0]->name,
                listas[0]->Xregisters[0], listas[0]->Xregisters[1],
                listas[0]->Xregisters[2], listas[0]->Xregisters[3],
                listas[1]->id, listas[1]->name,
                listas[1]->Xregisters[0], listas[1]->Xregisters[1],
                listas[1]->Xregisters[2], listas[1]->Xregisters[3]);
                
            mvwprintw(SC->wind[4], fila + 1, 1,
                "                           PC[%4d] IR[%-20s]     "
                "                  PC[%4d] IR[%s]",
                listas[0]->PC, listas[0]->IR,
                listas[1]->PC, listas[1]->IR);
            
            // Segundos dos
            fila += 3;  //  IMPORTANTE: Incrementar fila
            mvwprintw(SC->wind[4], fila, 1,
                "-->>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d]       "
                "-->>[ID:%4d] [%10s] AX[%4d] BX[%4d] CX[%4d] DX[%4d]",
                listas[2]->id, listas[2]->name,
                listas[2]->Xregisters[0], listas[2]->Xregisters[1],
                listas[2]->Xregisters[2], listas[2]->Xregisters[3],
                listas[3]->id, listas[3]->name,
                listas[3]->Xregisters[0], listas[3]->Xregisters[1],
                listas[3]->Xregisters[2], listas[3]->Xregisters[3]);
                
            mvwprintw(SC->wind[4], fila + 1, 1,
                "                           PC[%4d] IR[%-20s]     "
                "                  PC[%4d] IR[%s]",
                listas[2]->PC, listas[2]->IR,
                listas[3]->PC, listas[3]->IR);
        }
    }
    else {
        mvwprintw(SC->wind[4], 2, 1, "not found in mode");
    }
    
    SC->windupdate[4] = 0;
    wrefresh(SC->wind[4]);
}

void printPROY2(SCREEN *screen){
    werase(screen->wind[2]);
    box(screen->wind[2], 0, 0);
    mvwprintw(screen->wind[2],1,1,"HOLA");
    screen->windupdate[2] = 0;
    wrefresh(screen->wind[2]);
    
}