#include "../procesos.h"

int pid;

int main(int argc, char const *argv[])
{
    int keyNodo;
    int quiero = 0;

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
    
    memoria_compartida *mem;


    key_t key = ftok("..",1);





    int permisos = 0666; // permisos de lectura/escritura para todos los usuarios
    int msg_memoria_id = shmget(key+keyNodo, sizeof(memoria_compartida), permisos | IPC_CREAT);
    mem = shmat(msg_memoria_id, NULL, 0);

    #ifdef __PRINT_PROCESO
    printf("Key: %i y id de la memoria compartida es %i\n",key+keyNodo,msg_memoria_id);
    #endif 





    while (1)
    {
        // Quiero entrar en la sección críticia
        // Compruebo que no hay procesos prioritários intentando entrar.

        sem_wait(&mem->sem_aux_variables);
        #ifdef __PRINT_PROCESO
        printf("Tenemos el libro\n");
        #endif 
        mem->procesos_c_pend ++; // Indicamos que un proceso de consulta desea entrar en la SC

        if (mem->procesos_r_pend == 0  && mem->procesos_a_pend == 0 && mem->procesos_p_a_pend == 0){ // En caso de que no haya procesos prioritarios intentando entrar en la SC
            mem->procesos_c ++; 
            if (mem->procesos_c == 1)
            {
                /* code */
                sem_wait(&mem->sem_mutex); // Intentamos entrar en la SC
                sem_post(&mem->sem_aux_variables); 
            }else{
                sem_post(&mem->sem_aux_variables);
            }


            // SECCIÓN CRÍTICA
            #ifdef __PRINT_SC
            printf("Haciendo la SC\n");

            printf("Fin de la SC\n");
            #endif 
            // FIN SECCIÓN CRÍTICA


            sem_wait(&mem->sem_aux_variables);
            mem->procesos_c --;
            mem->procesos_c_pend --;
            if (mem->procesos_c == 0){ // Si ya no hay procesos de consulta ejecutandose se sale de la exclusion mutua
                sem_post(&mem->sem_mutex); // Hacemos que otros procesos puedan entrar en la seccion critica
            }
            sem_post(&mem->sem_aux_variables);
        }
        else{
            if (quiero == 0) {
                mem->procesos_c_pend ++;
                quiero = 1;
            }
            sem_post(&mem->sem_aux_variables); 
            sem_wait(&mem->sem_cosultas); // Esperamos a que nos permitan realizar una consulta
        }
    }
    
    return 0;
}
