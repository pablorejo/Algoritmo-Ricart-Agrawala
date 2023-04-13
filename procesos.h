#ifndef __PROCESOS_H
#define __PROCESOS_H


#include <stdio.h> // para los printf
#include <unistd.h> // para el sleep
#include <sys/msg.h> // Para usar la funcion maso de mensajes
#include <stdlib.h> // Para el atoi


// OPCIONES DE LOS SEMAFOROS
#define SEM_MUTEX 1
#define SEM_SYNC_INIT 2
#define SEM_SYNC_END 3
#define SEM_SYNC_INTENTAR 4


// ID del nodo controlador para dar de alta y de baja a los nodos
#define ID_NODO_CONTROLADOR 1




// Opciones de los mensajes
#define ACK 0
#define DAR_BAJA_NODO 0 
#define DAR_ALTA_NODO 1
#define DADO_DE_ALTA 2



typedef struct
{
    long mtype; 
}semaforo;

typedef struct 
{
    long mtype ; // Donde guardaremos el nodo origen
    int id_origen; // Irigen del mensaje
    int ticket_origen; // ticket de origen
    int opcion; // Opcion para dar de alta y de baja a los nodos
    int id_nodos[]; // Array con todos los nodos para poder enviarlos 
}mensaje;


#define N 1000 //Numero maximo de procesos y de nodos en el sistema
#define SLEEP 3 // Tiempo de espera para poder ver bien lo que hace



#define __PRINT // Comentar en caso de que no se quiera imprimir mensajes
#define __PRINT_SC // comentar en caso de que no se quiera ver si los proceso estan o no en la sección crítica



#endif