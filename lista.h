#ifndef COLA_H
#define COLA_H
#pragma pack(1)

typedef struct nodo {
    int id;
    char name[200];
    int Xregisters[4];
    int PC;
    char IR[32];
    struct nodo *sig;
}NODO;

void imprimir_lista(struct nodo *);

NODO * crear_nodo (int n,char name[]);

void insertar_nodo (NODO **, NODO *);

void eliminar_nodo (NODO **);

NODO * extraernodoN (NODO ** lista);

void vaciar_lista(NODO **);

NODO * extraer (NODO ** lista);

#endif