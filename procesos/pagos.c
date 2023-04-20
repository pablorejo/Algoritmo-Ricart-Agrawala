#include "../procesos.h"

int pid;


memoria_compartida *mem;


int main(int argc, char const *argv[])
{
    int keyNodo;
    if (argc < 2){
        // #ifdef __PRINT_PROCESO
        //     printf("Error introduce el numero de id de su nodo");
        // #endif // DEBUG

        // exit(-1);
        keyNodo = 1;

    }else {
        keyNodo = atoi(argv[1]); // tiene que ser igual a la id del nodo
    }

    

    pid = getpid();

    #ifdef __PRINT_PROCESO
    printf("Soy el proceso con pid %i\n",pid);
    #endif 
    


    key_t key = ftok(".",1);





    int permisos = 0666; // permisos de lectura/escritura para todos los usuarios
    int msg_memoria_id = shmget(key+keyNodo, sizeof(memoria_compartida), permisos | IPC_CREAT);
    mem = shmat(msg_memoria_id, NULL, 0);

    #ifdef __PRINT_PROCESO
    printf("Key: %i y id de la memoria compartida es %i\n",key,msg_memoria_id);
    #endif 


    while (1){
        // Quiero entrar en la sección críticia
        // Compruebo que no hay procesos prioritários intentando entrar.
        

        sem_wait(&mem->sem_aux_variables);
        mem->procesos_p_a_pend ++; // Indicamos que el de pagos desea entrar
        sem_post(&mem->sem_aux_variables); 

        #ifdef __PRINT_PROCESO
        printf("Intentando entrar en la seccion critica\n");
        #endif // DEBUG


        sem_post(&mem->sem_sync_intentar); // Inentamos entrar en la seccion 
       

        int valor;
        sem_getvalue(&mem->sem_sync_intentar,&valor);
        printf("%i\n",valor);

        printf("Enviando señal a recibir\n");
        sem_wait(&mem->sem_pagos_anulaciones); // Nos dejan entrar en la SC


        // SECCIÓN CRÍTICA
        #ifdef __PRINT_SC
        printf("Haciendo la SC\n");

        printf("Fin de la SC\n");
        #endif 
        // FIN SECCIÓN CRÍTICA


        sem_wait(&mem->sem_aux_variables);
        mem->procesos_p_a_pend --;
        sem_post(&mem->sem_aux_variables); 

        sem_post(&mem->sem_sync_end);// Hacemos que otros procesos puedan entrar en la seccion critica
        // Avisamos al proceso recibir de que hemos terminado la seccion critica
    }
    return 0;
}
