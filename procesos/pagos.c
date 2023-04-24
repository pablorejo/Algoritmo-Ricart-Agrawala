#include "../procesos.h"

int pid;

memoria_compartida *mem;


void enviar_tickets(int pri); 

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
    





    key_t key = ftok(".",1);
    msg_tickets_id = msgget(key,0660 | IPC_CREAT); // Creamos el buzón

    int permisos = 0666; // permisos de lectura/escritura para todos los usuarios
    int msg_memoria_id = shmget(key+keyNodo, sizeof(memoria_compartida), permisos | IPC_CREAT);
    mem = shmat(msg_memoria_id, NULL, 0);


    if (mem->n_nodos > 0)
    {
        for (int i = 0; i < mem->n_nodos; i++)
        {
            id_nodos[i] = i;
        }
    }else{
        printf("Tiene que haber nodos ejecutandose\n");
        exit(-1);
    }



    #ifdef __PRINT_PROCESO
    printf("Key: %i y id de la memoria compartida es %i\n",key,msg_memoria_id);
    #endif 


    while (1){
        // Quiero entrar en la sección críticia
        // Compruebo que no hay procesos prioritários intentando entrar.
        

        sem_wait(&(mem->sem_aux_variables));
        mem->procesos_p_a_pend ++; // Indicamos que el de pagos desea entrar

        #ifdef __PRINT_PROCESO
        printf("Intentando entrar en la seccion critica\n");
        #endif // DEBUG

        if (mem->tenemos_SC == 0)
        {
            enviar_tickets(PAGOS_ANULACIONES);
            printf("Hemos enviado los tickets\n");
        }
        sem_post(&(mem->sem_aux_variables)); 
        


        sem_wait(&(mem->sem_pagos_anulaciones)); // Nos dejan entrar en la SC


        // SECCIÓN CRÍTICA
        #ifdef __PRINT_SC
        printf("Haciendo la SC\n");
        sleep(SLEEP);
        printf("Fin de la SC\n");
        #endif 
        // FIN SECCIÓN CRÍTICA


        sem_wait(&(mem->sem_aux_variables));
        mem->procesos_p_a_pend --;
        sem_post(&(mem->sem_aux_variables)); 

        sem_post(&(mem->sem_sync_end));// Hacemos que otros procesos puedan entrar en la seccion critica
        // Avisamos al proceso recibir de que hemos terminado la seccion critica

        // sleep(SLEEP);s
    }
    return 0;
}


void enviar_tickets(int pri){
    mensaje msg_tick;
    sem_wait(&(mem->sem_aux_variables));
    mem->quiero = 1;
    mem->mi_ticket = mem->max_ticket + 1;
    sem_post(&(mem->sem_aux_variables));

    for (int i = 0; i < mem->n_nodos; i++) {
        //Enviamos un mensaje a todos los nodos diciendo que queremos entrar en la sección crítica
        if (id_nodos[i] != mem->mi_id){
            msg_tick.mtype = id_nodos[i];
            msg_tick.id_origen = mem->mi_id;
            msg_tick.ticket_origen = mem->mi_ticket;
            msg_tick.prioridad = pri; //Establecemos el mensaje de envio como la prioridad maxima entre nuestros procesos
            msgsnd(msg_tickets_id, &msg_tick, sizeof(mensaje), 0); //Enviamos el ticket al nodo 
            
            #ifdef __PRINT_RECIBIR
            printf("Enviando el mensaje %i al nodo %li desde el nodo %li\n",msg_tick.ticket_origen,msg_tick.mtype,mem->mi_id);
            #endif // DEBUG
        }
    }
}

