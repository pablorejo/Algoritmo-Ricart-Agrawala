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

#include <time.h>
#define ACK 0

#define ID_NODO_CONTROLADOR 1
#define N 1000 //Numero maximo de procesos y de nodos en el sistema
#define SLEEP 1 // Tiempo de espera para poder ver bien lo que hace



#define __PRINT_RECIBIR // Comentar en caso de que no se quiera imprimir mensajes del proceso recivir
#define __PRINT_PROCESO // Comentar en caso de que no se quiera imprimir mensajes de los otros procesos
#define __PRINT_SC // comentar en caso de que no se quiera ver si los proceso estan o no en la sección crítica
#define __PRINT_CTRL_C // comentar en caso de que no se quiera imprimir mensajes de control de terminar un mensaje
#define __BUCLE // Para que haga los procesos en bucle
// #define __RECABAR_DATOS // Para que guarde los datos en ficheros

#define DEBUG // Descomentar en caso de que no se tenga que pasar parametros
#define CARPETA "/home/pio"


// Prioridades de los procesos
// Cuanto mayor sea el numero mas prioridad tendra
#define PAGOS_ANULACIONES 5
#define ADMINISTRACION_RESERVAS 3
#define CONSULTAS 1


// definimos el numero maximo de ejecuciones que se pueden hacer de una misma prioridad en un nodo si hay nodos pidiendo con la misma prioridad
#define N_MAX_INTENTOS 3

// #define PROCESO_SYNC 1 // No lo estamos usando suponemos nodos estáticos

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
    // No cambian una vez establecidas
    long mi_id; int n_nodos;
    int id_nodos[N-1];

    // Arrays de los ids de nodos pendientes
    int id_nodos_pend_pagos_anulaciones[N-1], id_nodos_pend_administracion_reservas[N-1], id_nodos_pend_consultas[N-1];
    int tickets_pend_pagos_anulaciones[N-1], tickets_pend_administracion_reservas[N-1], tickets_pend_consultas[N-1]; // Aqui guardamos los tickets pendientes para comprobar su prioridad

    // Memoria compartida
    int quiero;

    sem_t sem_mutex; // Semaforo de exclusion mutua
    // int procesos_p_a_esp, procesos_a_r_esp; // Procesos esperando a pasar a la cola

    int mi_ticket,max_ticket;

    // sem_t sem_aux_variables; // Semáforo axiliar para chequear la memoria compartida
    // sem_t sem_mutex; // Semaforo de exclusion mutua con todos los procesos menos los de consultas entre siç
    sem_t sem_paso_pagos_anulaciones, sem_paso_administracion_reservas, sem_paso_consultas; // Semaforos de paso 
    int pend_pagos_anulaciones, pend_administracion_reservas, pend_consultas;
    int prioridad_max_enviada;
    int intentos;
    int tenemos_SC; // Variable para comprovar si nuestro nodo tiene la seccion critica

    // Para las consultas
    int n_consultas, esperando_consultas, esperando; // Saber cuantas consultas están en SC;
    sem_t sem_pro_n_consultas, sem_ctrl_paso_consultas;

    //// Para los nodos
    int nodos_pend_pagos_anulaciones, nodos_pend_administracion_reservas, nodos_pend_consultas;
    int ack_pend_pagos_anulaciones, ack_pend_administracion_reservas, ack_pend_consultas;
    // int ack_numero_1_administracio_reservas; // Este nos servirá para saber si es el primer ack recibido para ese mismo nodo, Se cambiará al enviar pagos o cambiar a pagos
    // int ack_numero_1_consultas; // Este nos servirá para saber si es el primer ack recibido para ese mismo nodo, Se cambiará al enviar pagos o cambiar a pagos o administracion

    // Para hacer las graficas

    #ifdef __RECABAR_DATOS
        double elapse_time_pagos_anulaciones[N*N]; // Array que contendrá todos los tiempos de los procesos de pagos y anulaciones
        double elapse_time_administracion_reservas[N*N]; // Array que contendrá todos los tiempos de los procesos de administracion y reservas
        double elapse_time_consultas[N*N]; // Array que contendrá todos los tiempos de los procesos de pagos y anulaciones

        sem_t sem_elapse_pagos_anulaciones, sem_elapse_administracion_reservas, sem_elapse_consultas;
        int num_elapse_pagos_anulaciones, num_elapse_administracion_reservas, num_elapse_consultas;
    #endif // DEBUG



    // sem_t sem_sync_enviar_ack;
    // sem_t sem_sync_siguiente; 
    // Fin memória compartida



    // Semaforos de proteccion de memoria compartida
    // sem_t sem_pro_pend, sem_pro_ack, sem_pro_tenemos_sc; // Semaforos inicializados a 1

    sem_t sem_aux_variables;

}memoria_compartida;








void enviar_tickets(int pri);
void dsiguiente();
void seccionCritica();
void catch_ctrl_c(int sig); // Esta funcion se encargará de capturar la señal de ctrl+c
void enviar_acks();
void ack(int id_nodos_pend[N-1],int tickets[N-1], int *nodos_pend, int prioridade);


int msg_tickets_id;
int memoria_id;
memoria_compartida *mem;
int detener; // Para detener los procesos normales





void enviar_tickets(int pri){

    // Enviamos los tickets para poder entrar en la sección crítica
    mensaje msg_tick;
    msg_tick.id_origen = mem->mi_id;

    sem_wait(&(mem->sem_aux_variables));
    
    mem->mi_ticket = mem->max_ticket + 1;

    mem->quiero = 1;

    msg_tick.ticket_origen = mem->mi_ticket;
    sem_post(&(mem->sem_aux_variables));

    msg_tick.prioridad = pri;

    


    #ifdef __PRINT_RECIBIR
        printf("\nEnviando tickets de prioridad %i\n",pri);    
    #endif
    for (int i = 0; i < mem->n_nodos; i++)
    {
        if (mem->id_nodos[i] != mem->mi_id){
            sem_wait(&(mem->sem_aux_variables));
            switch (pri)
            {
            case PAGOS_ANULACIONES:
                mem->ack_pend_pagos_anulaciones ++;
                break;
            case ADMINISTRACION_RESERVAS:
                mem->ack_pend_administracion_reservas ++;
                break;
            case CONSULTAS:
                mem->ack_pend_consultas ++;
                break;
            default:
                break;
            }
            mem->prioridad_max_enviada = pri;
            sem_post(&(mem->sem_aux_variables));
            msg_tick.mtype = mem->id_nodos[i]; // Solo hace falta cambiar este parte del codigo de tal forma que irá mas rápido
            msgsnd(msg_tickets_id, &msg_tick, sizeof(mensaje), 0); //Enviamos el mensaje al nodo origen
        }
    }
}

void siguiente(){
    sem_wait(&(mem->sem_aux_variables));
    #ifdef __PRINT_RECIBIR
        // printf("pend_pagos_anulaciones: %i\npend_administracion_reservas: %i\npend_consultas: %i\nnodos_pend_pagos_anulaciones: %i\nnodos_pend_administracion_reservas: %i\nnodos_pend_consultas: %i\n", mem->pend_pagos_anulaciones,mem->pend_administracion_reservas,mem->pend_consultas,mem->nodos_pend_pagos_anulaciones,mem->nodos_pend_administracion_reservas,mem->nodos_pend_consultas);
        printf("Siguiente\n");
    #endif // DEBUG

    if (mem->pend_pagos_anulaciones > 0 && mem->n_consultas == 0){
        if (mem->nodos_pend_pagos_anulaciones > 0){
            if (mem->intentos == 0){
                mem->tenemos_SC = 0;
                mem->intentos = N_MAX_INTENTOS;
                sem_post(&(mem->sem_aux_variables));
                enviar_acks(-1);// No dejamos pasar a mas procesos de pagos y hacemos que se envien los ack
                enviar_tickets(PAGOS_ANULACIONES);
                #ifdef __PRINT_RECIBIR
                    printf("Enviando tickets");
                #endif
                
            }else {
                mem->intentos --;
                sem_post(&(mem->sem_aux_variables));
                sem_post(&(mem->sem_paso_pagos_anulaciones)); // Dejamos pasar a otro proceso de pagos
            }
        }else {
            mem->intentos = N_MAX_INTENTOS;
            sem_post(&(mem->sem_aux_variables));
            sem_post(&(mem->sem_paso_pagos_anulaciones)); // Dejamos pasar a otro proceso de pagos
            #ifdef __PRINT_RECIBIR
                printf("\nPagos en el nodo sin pagos pend en otros nodos\n");
            #endif
        }
    }else if (mem->nodos_pend_pagos_anulaciones > 0 && mem->n_consultas == 0){
        mem->intentos = N_MAX_INTENTOS;
        mem->tenemos_SC = 0;


        if (mem->pend_administracion_reservas > 0){
            sem_post(&(mem->sem_aux_variables));
            enviar_tickets(ADMINISTRACION_RESERVAS);
        }else if(mem->pend_consultas > 0){
            sem_post(&(mem->sem_aux_variables));
            enviar_tickets(CONSULTAS);
        }else {
            mem->prioridad_max_enviada = 0;
            mem->quiero = 0;
            sem_post(&(mem->sem_aux_variables));
        }
        enviar_acks(-1);// No dejamos pasar a mas procesos;

     }else if (mem->pend_administracion_reservas > 0 && mem->n_consultas == 0){
        #ifdef __PRINT_RECIBIR
            printf("pend_administracion_reservas > 0\n");
        #endif
        if (mem->nodos_pend_administracion_reservas){
            if (mem->intentos == 0){
                mem->tenemos_SC = 0;
                mem->intentos = N_MAX_INTENTOS;
                sem_post(&(mem->sem_aux_variables));
                enviar_acks(-1);// No dejamos pasar a mas procesos de pagos y hacemos que se envien los ack
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
    }else if (mem->nodos_pend_administracion_reservas > 0 && mem->n_consultas == 0){
        #ifdef __PRINT_RECIBIR
            printf("nodos_pend_administracion_reservas > 0\n");
        #endif
        mem->intentos = N_MAX_INTENTOS;
        mem->tenemos_SC = 0;
        if(mem->pend_consultas > 0){
            sem_post(&(mem->sem_aux_variables));
            enviar_tickets(CONSULTAS);
        }else {
            mem->prioridad_max_enviada = 0;
            mem->quiero = 0;
            sem_post(&(mem->sem_aux_variables));
        }
        enviar_acks(-1);
    }else{
        if (mem->pend_consultas > 0){
            if (mem->esperando == 1)
            {
                sem_post(&(mem->sem_paso_consultas));
            }else{
                sem_post(&(mem->sem_ctrl_paso_consultas));
            }
            mem->prioridad_max_enviada = CONSULTAS;
        }else{
            mem->prioridad_max_enviada = 0;
            mem->quiero = 0;
            mem->tenemos_SC = 0;
            #ifdef __PRINT_RECIBIR
                printf("No hay procesos pendientes en el nodo\n");
            #endif
        }

        if (mem->nodos_pend_consultas>0){
            sem_post(&(mem->sem_aux_variables));
            enviar_acks(-1);
        }else{
            sem_post(&(mem->sem_aux_variables));
        }
        #ifdef __PRINT_RECIBIR
            printf("\nNo hay nada\n");
        #endif
    }
    #ifdef __PRINT_RECIBIR
        printf("Fin siguiente\n\n");
    #endif
}

void seccionCritica(){
    sleep(SLEEP);
    printf("Haciendo la SC\n");
    sleep(SLEEP);
    printf("Fin de la SC\n");
    sleep(SLEEP);
}



void enviar_acks(){

    #ifdef __PRINT_RECIBIR
        printf("Vamos a enviar ACKs\n");
    #endif

    sem_wait(&(mem->sem_aux_variables));
    if (mem->quiero == 0){
        ack(mem->id_nodos_pend_pagos_anulaciones,mem->tickets_pend_pagos_anulaciones, &mem->nodos_pend_pagos_anulaciones, PAGOS_ANULACIONES);
        ack(mem->id_nodos_pend_administracion_reservas,mem->tickets_pend_administracion_reservas, &mem->nodos_pend_administracion_reservas, ADMINISTRACION_RESERVAS);
        ack(mem->id_nodos_pend_consultas,mem->tickets_pend_consultas, &mem->nodos_pend_consultas, CONSULTAS);
        #ifdef __PRINT_RECIBIR
            printf("Quiero = 0\n");
        #endif
    }else{
        
        if (mem->nodos_pend_pagos_anulaciones > 0){
            ack(mem->id_nodos_pend_pagos_anulaciones,mem->tickets_pend_pagos_anulaciones, &mem->nodos_pend_pagos_anulaciones, PAGOS_ANULACIONES);
            #ifdef __PRINT_RECIBIR
                printf("Enviando ack a los nodos de tipo pagos o anulaciones\n");
            #endif
            if (mem->nodos_pend_administracion_reservas > 0 && mem->pend_pagos_anulaciones == 0) 
            {
                ack(mem->id_nodos_pend_pagos_anulaciones,mem->tickets_pend_administracion_reservas, &mem->nodos_pend_administracion_reservas, ADMINISTRACION_RESERVAS);
                if(mem->nodos_pend_consultas > 0 && mem->pend_administracion_reservas == 0){
                    ack(mem->id_nodos_pend_consultas,mem->tickets_pend_consultas, &mem->nodos_pend_consultas, CONSULTAS);
                }
            }else if ( mem->nodos_pend_consultas > 0 && mem->pend_administracion_reservas == 0){
                ack(mem->id_nodos_pend_consultas,mem->tickets_pend_consultas, &mem->nodos_pend_consultas, CONSULTAS);
            }
            
            
        }else if(mem->nodos_pend_administracion_reservas > 0){
            ack(mem->id_nodos_pend_administracion_reservas,mem->tickets_pend_administracion_reservas, &mem->nodos_pend_administracion_reservas, ADMINISTRACION_RESERVAS);
            #ifdef __PRINT_RECIBIR
                printf("Enviando ack a los nodos de tipo administracion o reservas\n");
            #endif  
            if ( mem->nodos_pend_consultas > 0 && mem->pend_administracion_reservas == 0){
                ack(mem->id_nodos_pend_consultas,mem->tickets_pend_consultas, &mem->nodos_pend_consultas, CONSULTAS);
            }

        }else if(mem->nodos_pend_consultas > 0){
            ack(mem->id_nodos_pend_consultas,mem->tickets_pend_consultas, &mem->nodos_pend_consultas, CONSULTAS);
            #ifdef __PRINT_RECIBIR
                printf("Enviando ack a los nodos de tipo consultas\n");
            #endif
        }
    }

    sem_post(&(mem->sem_aux_variables));
    
}

// Funcion para enviar los ack a los distintos nodos de una misma prioridad
void ack(int id_nodos_pend[N-1],int tickets[N-1], int *nodos_pend, int prioridade){
    mensaje msg_tick;
    msg_tick.id_origen = mem->mi_id;
    msg_tick.ticket_origen = ACK;
    msg_tick.prioridad = prioridade;
    
    for (int i = 0; i < *nodos_pend; i++){
        // Enviamos los mensajes que nos quedasen pendientes de enviar
        if(mem->quiero == 0 || mem->mi_ticket > tickets[i] || (mem->mi_ticket == tickets[i] && mem->mi_id > id_nodos_pend[i])){
            *nodos_pend = *nodos_pend - 1;
            msg_tick.mtype = id_nodos_pend[i];
            msgsnd(msg_tickets_id, &msg_tick, sizeof(mensaje), 0); //Enviamos el mensaje al nodo origen
            #ifdef __PRINT_RECIBIR
                printf("Enviando el ack de prioridad %i al nodo %li desde el nodo %li con prioridad %i\n",prioridade,msg_tick.mtype,mem->mi_id,prioridade);
            #endif // DEBUG
        }
    }
    
}


#endif

