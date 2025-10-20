#ifndef _PROY2_
#define _PROY2_
#include <pthread.h>
#include <semaphore.h>  // 

// Forward declarations
typedef struct screen SCREEN;
typedef struct bff BUFFER;
typedef struct cli CLI;
typedef struct nodo NODO;

typedef struct core{
    int intructionsID;

    int IDproccesor;
    int cuantum;
    int completedcycles;
    long cyclecounter;
    long maxCycles;
    int executiongmode;    

    NODO *LISTESPERA;
    NODO *LISTEJECUCION[4];

    pthread_t thread_FOR_list[4];
    pthread_t thread_FOR_instructions[4];

    pthread_mutex_t mutex;
    
}CORE;

typedef struct thread_args{
    NODO **LISTAEJEC;
    NODO **LISTAESPE;
    int numero_nodo;
    pthread_mutex_t *mutex;

    SCREEN *sc;
    BUFFER *bff;
    CLI *cli;
    CORE *core;
    int n_lista;
}THR_ARG;

void comandPROY2(SCREEN *sc, BUFFER *bff, CLI *cli, CORE *core);
void initializecore(CORE *core);
void execution_processes(SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core, int i);
void verifyexecution(SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core);
void frecuency_execution(SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core);
int load_instructions (SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core, int n_lista);
char *leer_renglon(const char *nombreArchivo, int n);



void execute_all_processes_threaded(SCREEN *sc, BUFFER *bff, CLI *cli, CORE *core);
void *thread_move_nodo(void *argumentos);
void *thread_instructions(void *argumentos);

int executeIsttruccion(SCREEN *sc,BUFFER *comand,CORE * core, CLI *cli,int n_lista);
int stringToUpper(char* str);
int stringToInteger(char* str);
int stringToRegisterIndex(char* str);
int instructionMOV(NODO* pcb, int p1, int p2, int mode);
int instructionADD(NODO* pcb, int p1, int p2, int mode);
int instructionSUB(NODO* pcb, int p1, int p2, int mode);
int instructionMUL(NODO* pcb, int p1, int p2, int mode);
int instructionDIV(NODO* pcb, int p1, int p2, int mode);
int instructionINC(NODO* pcb, int p1, int p2, int mode);
int instructionDEC(NODO* pcb, int p1, int p2, int mode);

void printLISTwaiting(SCREEN * SC, CLI *cli,CORE *c);
void printLISTexecute(SCREEN * SC, CLI *cli,CORE *c);
void printPROY2(SCREEN *screen);

#endif