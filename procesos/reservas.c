#include "../procesos.h"

int pid;

int main(int argc, char const *argv[])
{
    int keyNodo;
    if (argc < 2){
        #ifdef __PRINT_PROCESO
            printf("Error introduce el numero de id de su nodo");
        #endif // DEBUG

        exit(-1);
        // keyNodo = 1;

    }else {
        keyNodo = atoi(argv[1]); // tiene que ser igual a la id del nodo
    }

    

    pid = getpid();

    #ifdef __PRINT_PROCESO
    printf("Soy el proceso con pid %i\n",pid);
    #endif 
    
    memoria_compartida *mem;


    key_t key = ftok(".",1);





    int permisos = 0666; // permisos de lectura/escritura para todos los usuarios
    int msg_memoria_id = shmget(key+keyNodo, sizeof(memoria_compartida), permisos | IPC_CREAT);
    mem = shmat(msg_memoria_id, NULL, 0);

    #ifdef __PRINT_PROCESO
    printf("Key: %i y id de la memoria compartida es %i\n",key+keyNodo,msg_memoria_id);
    #endif 


    while (1){
        // Quiero entrar en la sección críticia
        // Compruebo que no hay procesos prioritários intentando entrar.

        sem_wait(&mem->sem_aux_variables);
        mem->procesos_a_r_pend ++; // Indicamos que el de pagos desea entrar
        sem_post(&mem->sem_aux_variables); 


        sem_post(&mem->sem_sync_intentar); // Inentamos entrar en la seccion critica
        sem_wait(&mem->sem_administracion_reservas); // Nos dejan entrar en la SC


        // SECCIÓN CRÍTICA
        #ifdef __PRINT_SC
        printf("Haciendo la SC\n");

        printf("Fin de la SC\n");
        #endif 
        // FIN SECCIÓN CRÍTICA


        sem_wait(&mem->sem_aux_variables);
        mem->procesos_a_r_pend --;
        sem_post(&mem->sem_aux_variables); 

        sem_post(&mem->sem_sync_end);// Hacemos que otros procesos puedan entrar en la seccion critica
        
            
    }
    return 0;
}
