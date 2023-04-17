#ifndef __PROCESOS_H
#define __PROCESOS_H


#include <stdio.h> // para los printf
#include <unistd.h> // para el sleep
#include <sys/msg.h> // Para usar la funcion maso de mensajes
#include <stdlib.h> // Para el atoi
#include <signal.h> // Para capturar el ctrl+c y así liberar la exclusión mutua en caso de que sea necesario

#include <semaphore.h>

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
    int prioridad; // Tendremos que pasar la prioridad del proceso 
}mensaje;

typedef struct
{
    long mtype ; // ID del proceso origen
    int tipo_de_proceso; // Tipo de proceso
}procesos;



#define N 1000 //Numero maximo de procesos y de nodos en el sistema
#define SLEEP 1 // Tiempo de espera para poder ver bien lo que hace



#define __PRINT_RECIBIR // Comentar en caso de que no se quiera imprimir mensajes del proceso recivir
#define __PRINT_PROCESO // Comentar en caso de que no se quiera imprimir mensajes de los otros procesos
#define __PRINT_SC // comentar en caso de que no se quiera ver si los proceso estan o no en la sección crítica
#define __PRINT_CTRL_C // comentar en caso de que no se quiera imprimir mensajes de control de terminar un mensaje


// Rangos de los tickets con prioridades
#define PAGOS_ANULACIONES 1 
#define ADMINISTRACION 2 
#define RESERVAS 3 
#define CONSULTAS 4 

#define PROCESO_SYNC 1
#endif