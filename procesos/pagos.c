#include "procesos.h"
#include <errno.h>

int pid;

int main(int argc, char const *argv[])
{
    detener = 0;
    int keyNodo;
    if (argc < 2){
        #ifndef DEBUG
            printf("Error introduce el numero de id de su nodo");
            exit(-1);   
        #endif // !DEBUG

        #ifdef DEBUG
            keyNodo = 1;
        #endif // DEBUG

    }else {
        keyNodo = atoi(argv[1]); // tiene que ser igual a la id del nodo
    }

    
    #ifdef  __RECABAR_DATOS
        struct timespec start, end;
    #endif // DEBUG

    


    #ifdef __PRINT_PROCESO
    pid = getpid();
    printf("Soy el proceso con pid %i\n",pid);
    #endif 
    
    #ifdef __BUCLE
    signal(SIGINT, &catch_ctrl_c); // Para saber cuando detener mi programa
    #endif

    // key_t key = ftok("../procesos_bin",1);
    key_t key = ftok(CARPETA,1);
    
    msg_tickets_id = msgget(key, 0660 | IPC_CREAT); // Creamos el buzón
    memoria_id = shmget(key+keyNodo, sizeof(memoria_compartida), 0660 | IPC_CREAT);
    
    #ifdef __PRINT_PROCESO
    printf("Key: %i y id de la memoria compartida es %i\n",key,memoria_id);
    #endif 

    if (memoria_id == -1){
        perror("Error al intentar crear la memoria compartida");
    }

    // memoria_id = 196609;
    mem = shmat(memoria_id, NULL, 0);


    if (mem->n_nodos <= 0){
        printf("Tiene que haber nodos ejecutandose\n");
        exit(-1);
    }

    #ifdef __BUCLE
    while (1){
    #endif
        // Quiero entrar en la sección críticia
        // Compruebo que no hay procesos prioritários intentando entrar.
        
        #ifdef __PRINT_PROCESO
            printf("Proceso de pagos en ejecucion 2\n");
        #endif 

        #ifdef __RECABAR_DATOS
            clock_gettime(CLOCK_REALTIME, &start);
        #endif

        sem_wait(&(mem->sem_aux_variables));


        mem->pend_pagos_anulaciones ++;
        
        if (mem->prioridad_max_enviada < PAGOS_ANULACIONES && (mem->tenemos_SC == 0 || mem->n_consultas > 0)){
            mem->quiero = 1;
            mem->prioridad_max_enviada = PAGOS_ANULACIONES;
            sem_post(&(mem->sem_aux_variables));
            enviar_tickets(PAGOS_ANULACIONES);
            #ifdef __PRINT_PROCESO
                printf("La prioridad enviada mas baja es menor que pagos o anulaciones\n");
            #endif 
            
        }else{
            sem_post(&(mem->sem_aux_variables));
        }
        

        #ifdef __PRINT_PROCESO
            printf("Intentando entrar en la seccion critica\n");
        #endif 

        sem_wait(&(mem->sem_paso_pagos_anulaciones)); // Nos dejan entrar en la SC

        // SECCIÓN CRÍTICA
        #ifdef __PRINT_SC
            seccionCritica();
        #endif 
        // FIN SECCIÓN CRÍTICA

        
        sem_wait(&(mem->sem_aux_variables));
        mem->pend_pagos_anulaciones --;
        sem_post(&(mem->sem_aux_variables));

        siguiente();

        #ifdef __PRINT_PROCESO
            printf("Fin pagos\n\n");
        #endif
        #ifdef __RECABAR_DATOS
            clock_gettime(CLOCK_REALTIME, &end);
            // Imprimir el tiempo que tarda en ejecutarse :)
            if (mem->num_elapse_pagos_anulaciones > N*N){
                printf("Terminando hemos superado el tamaño maximo del array");
                exit(0);
            }

            double time_taken = (end.tv_sec - start.tv_sec) * 1e3;
            time_taken += (end.tv_nsec - start.tv_nsec) / 1e6;

            
            sem_wait(&(mem->sem_elapse_pagos_anulaciones));
            mem->elapse_time_pagos_anulaciones[mem->num_elapse_pagos_anulaciones] = time_taken;
            mem->num_elapse_pagos_anulaciones ++;
            sem_post(&(mem->sem_elapse_pagos_anulaciones));


            #ifdef __PRINT_PROCESO
                printf("time taken: %f ms\n\n",time_taken);
            #endif
        #endif // DEBUG



        #ifdef __BUCLE 
        if (detener == 1){
            exit(0);
        }
    }
    #endif
    return 0;
}


#ifdef __BUCLE
void catch_ctrl_c(int sig){
    detener = 1;
}
#endif
