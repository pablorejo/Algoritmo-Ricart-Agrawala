#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/types.h> // Se definen algunos tipos de datos
#include <sys/ipc.h> // Para algunas banderas
#include <sys/msg.h> // Para usar la funcion

#include <semaphore.h>

#include <unistd.h>




#include "procesos.h" // Incluimos la cabecera de los procesos

int pid;
semaforo *semaforos;


int main(int argc, char const *argv[])
{
    pid = getpid();
    printf("Soy el proceso con pid %i\n",pid);


    key_t key = ftok("recibir.c",213);
    semaforos_id = shmget(key, sizeof(semaforo), IPC_CREAT | 0666);
    semaforos = (semaforo *)shmat(semaforos_id, NULL, 0);
    


    while (1)
    {
        while (getchar() != '\n') {} // Esperamos a que se introduzca un enter
        printf("El proceso %i está intentando entrar en la SC\n",pid);

        sem_wait(&semaforos->sem_mutext); // Impidimos que otros procesos (mesmo nodo) puedan entrar en la seccion crítica
        printf("Intentando entrar\n");
        int v_sem_mutex;
        sem_getvalue(&semaforos->sem_sync_intentar,&v_sem_mutex);
        printf("%i\n",v_sem_mutex);
        sem_post(&semaforos->sem_sync_intentar); // Intentamos entrar en la seccion critica
        sem_getvalue(&semaforos->sem_sync_intentar,&v_sem_mutex);
        printf("%i\n",v_sem_mutex);
        sem_wait(&semaforos->sem_sync_init); // Recivimos sincronizacion para entrar en la seccion critcia
        
        // SECION CRÍTICA
        printf("El proceso %i está en la sección crítica\n",pid);
        while (getchar() != '\n') {} // Esperamos a que se introduzca un enter
        printf("Realizando la seccion críticia\n");
        sleep(SLEEP);
        while (getchar() != '\n') {} // Esperamos a que se introduzca un enter
        printf("El proceso %i abandonó la sección crítica\n",pid);
        // TERMINA LA SECCIÓN CRÍTICA

        sem_post(&semaforos->sem_sync_end); // Enviamos sincronizacion de que terminamos la seccion crítica
        sem_post(&semaforos->sem_mutext); // Permitimos a otros procesos del nodo entrar en la seción crítica
        /* code */
    }
    
    return 0;
}
