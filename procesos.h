#ifndef __PROCESOS_H
#define __PROCESOS_H


#include <stdio.h> // para los printf
#include <unistd.h> // para el sleep
#include <sys/msg.h> // Para usar la funcion maso de mensajes
#include <stdlib.h> // Para el atoi



#define SEM_MUTEX 1
#define SEM_SYNC_INIT 2
#define SEM_SYNC_END 3
#define SEM_SYNC_INTENTAR 4
#define ACK 0

#define ID_NODO_CONTROLADOR 1

typedef struct
{
    long mtype; 
}semaforo;

typedef struct 
{
    long mtype ; // Donde guardaremos el nodo origen
    int id_origen;
    int ticket_origen;
}mensaje;


#define N 1000 //Numero maximo de procesos y de nodos en el sistema
#define SLEEP 3 // Tiempo de espera para poder ver bien lo que hace



#define __PRINT_RECIBIR // Comentar en caso de que no se quiera imprimir mensajes del proceso recivir
#define __PRINT_PROCESO // Comentar en caso de que no se quiera imprimir mensajes de los otros procesos
#define __PRINT_SC // comentar en caso de que no se quiera ver si los proceso estan o no en la sección crítica

#endif