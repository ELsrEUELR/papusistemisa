#include <stdio.h>
#include <stdlib.h>
#include"lista.h"

int menu(void)
{
    int opcion, num;
    printf("\nMenu\n");
    printf("---------------\n");
    printf("1.-AGREGAR-----\n");
    printf("2.-QUITAR------\n");
    printf("3.-IMPRIMIR----\n");
    printf("4.-VACIAR------\n");
    printf("5.-SALIR-------\n");
    printf("---------------\n");
    do{
    num = scanf("%d", &opcion);
        if (num == 1) {
            break;
        } 
        else {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Entrada inválida.\ningrese un número entero.\n");
        }
    }while(1);
    return opcion;
}



int main(void)
{
    int opcion, num, elim;
    int resp;
    struct nodo *nodo = NULL;
    struct nodo *lista = NULL;
    do
    {
        opcion = menu();
        switch (opcion)
        {
        case 1:
            printf("Agregar\n");
            printf("Ingresa el id: ");
             do{
                num = scanf("%d", &resp);
                if (num == 1) {
                    break;
                } 
                else {
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                    printf("Entrada inválida.\ningrese un número entero:");
                }
            }while(1);
            nodo = crear_nodo(resp);
            insertar_nodo(&lista, nodo);
            break;
        case 2:
            printf("Quitar\n");
            do{
                num = scanf("%d", &elim);
                if (num == 1) {
                    break;
                } 
                else {
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                    printf("Entrada inválida.\ningrese un número entero:");
                }
            }while(1);
            nodo = extraer_nodo(&lista, elim);
            if(nodo != NULL){
                printf("El nodo extraido tiene el id: %d",nodo -> id );
                eliminar_nodo(&nodo);
            }else{
                printf("lista Vacía o elemento no encontrado\n");
            }
            break;
        case 3:
            printf("Imprimir\n ");
            imprimir_lista(lista);
            break;
        case 4:
            printf("Vaciando lista\n");
            vaciar_lista(&lista);
            printf("...\n");
            printf("...\n");
            printf("lista vaciada con éxito.\n");
            break;
        case 5: 
            printf("Saliendo...\n");
            vaciar_lista(&lista);
            break;
        default:
            printf("Escoja una opción valida\n");
        }
    } while (opcion != 5);
    return 0;
}

