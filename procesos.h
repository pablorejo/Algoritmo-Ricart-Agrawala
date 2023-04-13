#ifndef __PROCESOS_H
#define __PROCESOS_H

#include <semaphore.h>
#include <sys/shm.h> // memoria compartida
#include <sys/stat.h> // memoria compartida
#include <string.h> // Cadenas de texto




#define SEM_MUTEX 1
#define SEM_SYNC_INIT 2
#define SEM_SYNC_END 3
#define SEM_SYNC_INTENTAR 4
#define ACK 0

typedef struct
{
    long mtype; 
}semaforo;




#define N 1000 //Numero maximo de procesos y de nodos en el sistema
#define SLEEP 3 // Tiempo de espera para poder ver bien lo que hace
 
#endif