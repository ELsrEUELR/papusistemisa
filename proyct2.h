#ifndef _PROY2_
#define _PROY2_

// Forward declarations (en lugar de incluir funtop.h)
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
    NODO *LISTEJECUCION;
}CORE;

void comandPROY2(SCREEN *sc, BUFFER *bff, CLI *cli, CORE *core);
void initializecore(CORE *core);
void execution_processes(SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core);
void verifyexecution(SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core);
void frecuency_execution(SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core);
int load_instructions (SCREEN *sc, BUFFER *bff, CLI *cli,CORE *core);
char *leer_renglon(const char *nombreArchivo, int n);


int executeIsttruccion(SCREEN *sc,BUFFER *comand,CORE * core, CLI *cli);
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

#endif