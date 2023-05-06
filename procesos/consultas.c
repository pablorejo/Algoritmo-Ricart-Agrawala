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

    
    printf("Hola\n");
    pid = getpid();

    #ifdef __PRINT_PROCESO
    printf("Soy el proceso con pid %i\n",pid);
    #endif 
    

    signal(SIGINT, &catch_ctrl_c); // Para saber cuando detener mi programa

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



    ///////// Aqui empieza el programa ////////////////
    while (1){
        // Quiero entrar en la sección críticia
        // Compruebo que no hay procesos prioritários intentando entrar.
        
        #ifdef __PRINT_PROCESO
            printf("Proceso de consultas en ejecucion\n");
        #endif 

        sem_wait(&(mem->sem_aux_variables));

        mem->pend_consultas ++;


        if (mem->prioridad_max_enviada < CONSULTAS && mem->tenemos_SC == 0 ){
            mem->quiero = 1;
            mem->prioridad_max_enviada = CONSULTAS;
            sem_post(&(mem->sem_aux_variables));
            enviar_tickets(CONSULTAS);
            #ifdef __PRINT_PROCESO
                printf("La prioridad enviada mas baja es menor que consultas\n");
            #endif 

            
        }else{
            
            sem_post(&(mem->sem_aux_variables));
        }
        
        #ifdef __PRINT_PROCESO
            printf("Intentando entrar en la seccion critica\n");
        #endif 

        sem_wait(&(mem->sem_aux_variables)); 
        printf("valor de la variable esperando consultas %i\n",mem->esperando_consultas);
        // Aqui podemos porner otro tipo de semaforo
        
        if (mem->esperando_consultas == 0){ // Comprobamos que no estamos esperando ya una consulta
            mem->esperando_consultas ++;
            #ifdef __PRINT_PROCESO
                printf("Soy el primer proceso de consultas\n");
            #endif 
            
            mem->esperando = 1;
            sem_post(&(mem->sem_aux_variables));
            sem_wait(&(mem->sem_paso_consultas));

            sem_wait(&(mem->sem_aux_variables));
            mem->esperando = 0;
            sem_post(&(mem->sem_aux_variables));    
            
        }else{ // Si ya estamos esperando por consultas esperamos a que la primera que esta esperando nos deje entrar
            #ifdef __PRINT_PROCESO
                printf("NO soy el primer proceso de consultas\n");
            #endif 
            mem->esperando_consultas ++;
            sem_post(&(mem->sem_aux_variables)); 
            sem_wait(&(mem->sem_ctrl_paso_consultas));
        }
        

        sem_wait(&(mem->sem_aux_variables)); 
        mem->n_consultas ++; 
        if ((mem->pend_pagos_anulaciones == 0 && mem->pend_administracion_reservas == 0 && mem->nodos_pend_pagos_anulaciones == 0 && mem->nodos_pend_administracion_reservas == 0) && mem->n_consultas < mem->pend_consultas){
            sem_post(&(mem->sem_aux_variables)); 
            sem_post(&(mem->sem_ctrl_paso_consultas)); // En caso de que no haya procesos prioritarios queriendo entrar y que los otros procesos 
        }else{
            sem_post(&(mem->sem_aux_variables)); 
        }

        
        // SECCIÓN CRÍTICA
        #ifdef __PRINT_SC
            seccionCritica();
        #endif
        // FIN SECCIÓN CRÍTICA


        sem_wait(&(mem->sem_aux_variables));
        mem->pend_consultas --;

        mem->n_consultas --;
        mem->esperando_consultas --;
        if (mem->n_consultas == 0){
            
            mem->prioridad_max_enviada = 0;
            sem_post(&(mem->sem_aux_variables));
            siguiente();
            printf("Fin de las consultas\n");
        }else{
            sem_post(&(mem->sem_aux_variables));
        }

        #ifdef __PRINT_PROCESO
            printf("Fin Consulta\n\n");
        #endif
        
        if (detener == 1){
            exit(0);
        }
    }
    return 0;
}


void catch_ctrl_c(int sig){
    detener = 1;
}

