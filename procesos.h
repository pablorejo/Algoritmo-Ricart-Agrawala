#ifndef __PROCESOS_H
#define __PROCESOS_H


#include <stdio.h> // para los printf
#include <unistd.h> // para el sleep
#include <sys/msg.h> // Para usar la funcion maso de mensajes
#include <stdlib.h> // Para el atoi
#include <signal.h> // Para capturar el ctrl+c y así liberar la exclusión mutua en caso de que sea necesario

#include <semaphore.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h> // Para memoria compartida

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


// Estrucutura de la memoria compartida
typedef struct 
{
    // Memoria compartida
    int quiero;
    int  procesos_c;
    int procesos_p_a_esp, procesos_a_r_esp; // Procesos esperando a pasar a la cola
    int procesos_p_a_pend, procesos_a_r_pend; // procesos esperando en la cola para ser atendidos
    long mi_id; // id del nodo actual
    int ack_enviados_p_a, ack_enviados_a_r;

    int mi_ticket,max_ticket;
    int n_nodos; // Esta variable nunca se cambia


    sem_t sem_aux_variables; // Semáforo axiliar para chequear la memoria compartida
    sem_t sem_mutex; // Semaforo de exclusion mutua con todos los procesos menos los de consultas entre siç
    sem_t sem_administracion_reservas, sem_pagos_anulaciones; // Semaforos de paso 
    int tenemos_SC; // Variable para comprovar si nuestro nodo tiene la seccion critica

    
    // Fin memória compartida

    // Semaforos de sincronizacion con el proceso recivir
    sem_t sem_sync_end;
    sem_t sem_sync_intentar;
    // Fin de los semaforos de sincronizacion con el proceso recivir



}memoria_compartida;




#define N 1000 //Numero maximo de procesos y de nodos en el sistema
#define SLEEP 1 // Tiempo de espera para poder ver bien lo que hace



#define __PRINT_RECIBIR // Comentar en caso de que no se quiera imprimir mensajes del proceso recivir
#define __PRINT_PROCESO // Comentar en caso de que no se quiera imprimir mensajes de los otros procesos
#define __PRINT_SC // comentar en caso de que no se quiera ver si los proceso estan o no en la sección crítica
#define __PRINT_CTRL_C // comentar en caso de que no se quiera imprimir mensajes de control de terminar un mensaje
#define DEBUG // Descomentar en caso de que no se tenga que pasar parametros


// Prioridades de los procesos
// Cuanto mayor sea el numero mas prioridad tendra
#define PAGOS_ANULACIONES 5
#define ADMINISTRACION_RESERVAS 3
#define CONSULTAS 1


// definimos el numero maximo de ejecuciones que se pueden hacer de una misma prioridad en un nodo si hay nodos pidiendo con la misma prioridad
#define N_MAX_INTENTOS 5

void enviar_tickets(int pri);
int id_nodos[N-1];
int msg_tickets_id;

#define PROCESO_SYNC 1
#endif