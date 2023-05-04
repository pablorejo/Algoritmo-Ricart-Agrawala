#include "../procesos.h"
#include <errno.h>

int pid;


void siguiente();


int main(int argc, char const *argv[])
{
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

    
    
    
    printf("Hola\n");
    pid = getpid();

    



    #ifdef __PRINT_PROCESO
    printf("Soy el proceso con pid %i\n",pid);
    #endif 
    





    // key_t key = ftok("../procesos_bin",1);
    key_t key = ftok(".",1);
    
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


    if (mem->n_nodos > 0)
    {
        for (int i = 0; i < mem->n_nodos; i++)
        {
            id_nodos[i] = i + 1;
        }
    }else{
        printf("Tiene que haber nodos ejecutandose\n");
        exit(-1);
    }





    while (1){
        // Quiero entrar en la sección críticia
        // Compruebo que no hay procesos prioritários intentando entrar.
        


        sem_wait(&(mem->sem_aux_variables));
        mem->pend_pagos_anulaciones ++;
        

        if (mem->prioridad_max_enviada < PAGOS_ANULACIONES)
        {
            mem->quiero = 1;
            mem->prioridad_max_enviada = PAGOS_ANULACIONES;
            sem_post(&(mem->sem_aux_variables));
            enviar_tickets(PAGOS_ANULACIONES);
            
        }else{
            sem_post(&(mem->sem_aux_variables));
        }
        

        #ifdef __PRINT_PROCESO
        printf("Intentando entrar en la seccion critica\n");
        #endif 

        sem_wait(&(mem->sem_paso_pagos_anulaciones)); // Nos dejan entrar en la SC


        // SECCIÓN CRÍTICA
        #ifdef __PRINT_SC
        printf("Haciendo la SC\n");
        sleep(SLEEP);
        printf("Fin de la SC\n");
        sleep(SLEEP);
        #endif 
        // FIN SECCIÓN CRÍTICA


        sem_wait(&(mem->sem_aux_variables));
        mem->pend_pagos_anulaciones --;
        sem_post(&(mem->sem_aux_variables));
        

        siguiente();
        
    }
    return 0;
}


void siguiente(){

    

    sem_wait(&(mem->sem_aux_variables));

    #ifdef __PRINT_RECIBIR
    // printf("pend_pagos_anulaciones: %i\npend_administracion_reservas: %i\npend_consultas: %i\nnodos_pend_pagos_anulaciones: %i\nnodos_pend_administracion_reservas: %i\nnodos_pend_consultas: %i\n", mem->pend_pagos_anulaciones,mem->pend_administracion_reservas,mem->pend_consultas,mem->nodos_pend_pagos_anulaciones,mem->nodos_pend_administracion_reservas,mem->nodos_pend_consultas);
    #endif // DEBUG


    if (mem->pend_pagos_anulaciones > 0){
        if (mem->nodos_pend_pagos_anulaciones > 0){
            if (mem->intentos == 0){
                mem->tenemos_SC = 0;
                mem->intentos = N_MAX_INTENTOS;
                sem_post(&mem->sem_sync_enviar_ack); // No dejamos pasar a mas procesos de pagos y hacemos que se envien los ack
                mem->prioridad_max_enviada = PAGOS_ANULACIONES;
                sem_post(&(mem->sem_aux_variables));
                enviar_tickets(PAGOS_ANULACIONES);
                printf("pagos\n");
                
            }else {
                mem->intentos --;
                sem_post(&(mem->sem_aux_variables));
                sem_post(&(mem->sem_paso_pagos_anulaciones)); // Dejamos pasar a otro proceso de pagos
                printf("pagos\n");
            }
        }else {
            mem->intentos = N_MAX_INTENTOS;
            sem_post(&(mem->sem_aux_variables));
            sem_post(&(mem->sem_paso_pagos_anulaciones)); // Dejamos pasar a otro proceso de pagos
            printf("pagos\n");
        }
    }else if (mem->nodos_pend_pagos_anulaciones > 0){
        printf("pagos\n");
        mem->tenemos_SC = 0;
        sem_post(&(mem->sem_aux_variables));
        sem_post(&mem->sem_sync_enviar_ack); // No dejamos pasar a mas procesos;
    }else if (mem->pend_administracion_reservas > 0){

        if (mem->nodos_pend_administracion_reservas){
            if (mem->intentos == 0){
                mem->tenemos_SC = 0;
                mem->intentos = N_MAX_INTENTOS;
                sem_post(&mem->sem_sync_enviar_ack); // No dejamos pasar a mas procesos de pagos y hacemos que se envien los ack
                mem->prioridad_max_enviada = ADMINISTRACION_RESERVAS;
                sem_post(&(mem->sem_aux_variables));
                enviar_tickets(ADMINISTRACION_RESERVAS);
            }else {
                mem->intentos --;
                sem_post(&(mem->sem_aux_variables));
                sem_post(&(mem->sem_paso_administracion_reservas)); // Dejamos pasar a otro proceso de pagos
            }
        }else {
            mem->intentos = N_MAX_INTENTOS;
            sem_post(&(mem->sem_aux_variables));
            sem_post(&(mem->sem_paso_administracion_reservas)); // Dejamos pasar a otro proceso de pagos
        }
    }else if (mem->nodos_pend_administracion_reservas > 0){
        mem->tenemos_SC = 0;
        sem_post(&(mem->sem_aux_variables));
        sem_post(&mem->sem_sync_enviar_ack);
    }else{
        mem->prioridad_max_enviada = CONSULTAS;
        if (mem->pend_consultas > 0){
            sem_post(&(mem->sem_paso_consultas));
        }
        if (mem->nodos_pend_consultas>0){
            sem_post(&mem->sem_sync_enviar_ack);
        }
        sem_post(&(mem->sem_aux_variables));
    }
   
}



// void reset_prioriti(){
//     if (mem->pend_pagos_anulaciones > 0){
//         mem->prioridad_max_enviada = PAGOS_ANULACIONES;
//         enviar_tickets(PAGOS_ANULACIONES);
//     }else if (mem->pend_administracion_reservas > 0){
//         mem->prioridad_max_enviada = ADMINISTRACION_RESERVAS;
//         enviar_tickets(ADMINISTRACION_RESERVAS);
//     }else if (mem->pend_consultas){
//         mem->prioridad_max_enviada = CONSULTAS;
//         enviar_tickets(CONSULTAS);
//     }else {
//         mem->prioridad_max_enviada = 0;
//     }
// }



