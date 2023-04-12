#ifndef __PROCESOS_H
#define __PROCESOS_H

#include <semaphore.h>
#include <sys/shm.h> // memoria compartida
#include <sys/stat.h> // memoria compartida
#include <string.h> // Cadenas de texto

typedef struct
{
    sem_t sem_mutext; // Semaforo de exclusion mutua para que no puedan entrar varios procesos a la sección crítica
    sem_t sem_sync_init; // Semáforo de sincronizacion para avisar al proceso recivir que se quiere entrar en la SC
    sem_t sem_sync_end; // Semáforo de sincronizacion para avisar al proceso recivir que se quiere salir de la SC
    sem_t sem_sync_intentar; // Semáforo para intentar entrar en la sección crítica
}semaforo;
// Todos los semáforos los inicializa el proceso recibir();
int semaforos_id;



#define N 1000
#define SLEEP 1

#endif