#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include"lista.h"

int contarNODO(NODO *lista){
    int n_nodos=0;
    while(lista != NULL){
        n_nodos++;
        lista = lista-> sig;
    }
    return n_nodos;
}

NODO * crear_nodo (int id,char name[]){
    NODO *nodo= NULL;
    nodo =(NODO *) malloc (sizeof (NODO));

    if (NULL != nodo){
        nodo-> id= id;
        strcpy(nodo->name, name);
        for(int i = 0; i < 4; i++){
            nodo->Xregisters[i] = 0;
        }
        nodo->PC = 0;
        nodo->IR[0] = '\0';
        nodo-> sig= NULL;
    }
    return nodo;
}

void insertar_nodo(NODO **lista, NODO *nodo){
    NODO *actual = *lista;
    NODO *anterior = NULL;
	if (*lista == NULL) {
        *lista = nodo;
        nodo->sig = NULL;
    }
    else{
        while(actual != NULL){
            anterior = actual;
            actual = actual->sig;
        }
        anterior->sig = nodo;
        nodo->sig = actual;
    } 
}

void eliminar_nodo (NODO **nodo){
    free(*nodo);
    *nodo = NULL;
}

NODO * extraernodoN (NODO ** lista){
	NODO *actual = *lista;
	NODO *anterior = NULL;
    NODO * nodo = NULL;
    if(actual !=NULL){
		while(actual != NULL){
			anterior = actual;
            actual = actual->sig;
		}
		if(actual == NULL){
			return NULL;
		}
		if(anterior == NULL){
            *lista = actual-> sig;
			actual-> sig = NULL;
		}
		else{
			anterior-> sig = actual-> sig;
			actual-> sig = NULL;		
		}
	}

	nodo = actual;
    return nodo;
}

NODO *extraer(NODO **lista) {
    NODO *aux = *lista; 
    if (aux == NULL) {
        return NULL;   
    }

    *lista = aux->sig;   
    aux->sig = NULL;     
    return aux;         
}

void vaciar_lista(struct nodo **lista) {
    while (*lista != NULL) {
        struct nodo *nodo = extraer(lista);
        eliminar_nodo(&nodo);
    }
}