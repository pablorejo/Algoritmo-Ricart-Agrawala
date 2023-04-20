#include <pthread.h> // Para hilos
#include <semaphore.h> // Para semáforos
#include "procesos.h" // Incluimos la cabecera de los procesos




int mi_ticket = 0, id_nodos[N-1] = {0}, quiero = 0, max_ticket = 0, n_nodos = N-1, ctrl_c = 0;

// Colas de pendientes de las prioridades recibidas de otros nodos
int num_pend_p_a = 0,num_pend_a_r = 0;
int id_nodos_pend_p_a[N-1] = {0}, id_nodos_pend_a_r[N-1] = {0};
int ack_enviados_p_a = 0 , ack_enviados_a_r = 0;

int msg_tickets_id; //id del buzón

long mi_id;
int procesos_pendientes = 0;
int prioridad_max_procesos;
int prioridad_max_recivida_nodos = 10000;



int max_intentos = N_MAX_INTENTOS;

// Buzones
mensaje msg_ticket;
semaforo msg_semaforo;


sem_t sem_mutex;
sem_t sem_ctrl_c;
pthread_t thread_enviar;
pthread_t thread_ctrl_c;

int memoria_id;
memoria_compartida *mem;

void recibir();
void* enviar(void *args);
void* fun_ctrl_c(void *args);
void catch_ctrl_c(int sig);
void ack(int id_nodos_pend[N-1], int *nodos_pend);
void enviar_acks();

int main(int argc, char const *argv[])
{
    if (argc < 3){
        #ifndef DEBUG
            printf("Introduce el id y cuantos nods hay\n");
            exit(-1);
        #endif // !DEBUG

        #ifdef DEBUG
            mi_id = 1; // Guardamos el id que nos otorgara el usuario    
            n_nodos = 2; // Numero de procesos totales
            // Guardando ids de los procesos
            for (int i = 0; i < n_nodos; i++){
                id_nodos[i] = i+1;
            }
        #endif // DEBUG

    }else{
        mi_id = atoi(argv[1]); // Guardamos el id que nos otorgara el usuario    
        n_nodos = atoi(argv[2]); // Numero de nodos totales
        
        
        for (int i = 0; i < n_nodos; i++){
            id_nodos[i] = i+1;
        }
    }
    
    #ifdef __PRINT_RECIBIR
    printf("Mi id es %li y el N de nodos es %i\n",mi_id,n_nodos);
    #endif // DEBUG



    key_t key = ftok(".",1);

    msg_tickets_id = msgget(key,0660 | IPC_CREAT); // Creamos el buzón


    // Memoria compartida
    

    memoria_id = shmget(key+mi_id, sizeof(memoria_compartida), 0666 | IPC_CREAT);
    mem = shmat(memoria_id, NULL, 0);
    ///////// Inicializamos la memoria compartida

    // Inicializamos los semaforos de exclusion mutua
    sem_init(&(mem->sem_aux_variables),1,1); // Semaforo para leer las variables de la memoria compartida
    printf("Hola\n");

    sem_init(&(mem->sem_mutex),1,1); // Semaforo para entrar en la seccion crítica

    // Semaforos de paso 
    sem_init(&(mem->sem_pagos_anulaciones),1,0);
    sem_init(&(mem->sem_administracion_reservas),1,0);


    // Inicializamos las variables a 0
    mem->tenemos_SC = 0;
    mem->procesos_a_r_pend = 0; mem->procesos_p_a_pend = 0;


    // Semaforos de sincronizacion con el proceso recivir
    sem_init(&(mem->sem_sync_end),1,0);
    sem_init(&(mem->sem_sync_intentar),1,0);
    int valor;
    sem_getvalue(&(mem->sem_sync_intentar),&valor);
    printf("%i\n",valor);
    ///////// Fin memoria compartida

    
    #ifdef __PRINT_RECIBIR
    printf("Key 1: %i e id del buzón %i\nID de la memoria compartida es %i\n",key,msg_tickets_id,memoria_id);

    #endif // DEBUG
 

    msg_ticket.mtype = mi_id;
    msg_ticket.ticket_origen = mi_ticket;

    // iniciamos los semáforos
    sem_init(&sem_mutex,1,1); // Semaforo de exclusión mutua para las variables
    sem_init(&sem_ctrl_c,1,0); // Semáforo de paso por si se desea cancelar la ejecucmax_ticket





    pthread_create(&thread_enviar, NULL, enviar, NULL);
    pthread_create(&thread_ctrl_c, NULL, fun_ctrl_c, NULL);
    // Controlar el ctrl+c
    signal(SIGINT, &catch_ctrl_c);

    recibir();
    return 0;
}


void* enviar(void *args) 
{

    #ifdef __PRINT_RECIBIR
    printf("Funcion enviar ok\n");
    #endif 

    
    while (1){

        #ifdef __PRINT_RECIBIR
        printf("Esperando semaforo\n");
        #endif 

        sem_wait(&(mem->sem_sync_intentar)); // Esperamos a recivir alguna peticion 
        // int valor;
        // sem_getvalue(&(mem->sem_sync_intentar),&valor);
        // printf("%i\n",valor);

        #ifdef __PRINT_RECIBIR
        printf("Intentando entrar a la sección crítica\n");
        #endif 


        // Semaforo de exclusión mutua aquí
        sem_wait(&sem_mutex);
        quiero = 1;
        mi_ticket = max_ticket + 1;
        sem_post(&sem_mutex);
        // Termina el semaforo

        #ifdef __PRINT_RECIBIR
        printf("Mi ticket es %i\n",msg_ticket.ticket_origen);
        #endif // DEBUG


        #ifdef __PRINT_RECIBIR
        printf("Enviando mensajes para a todos los nodos\n");
        #endif 
        

        sem_wait(&(mem->sem_aux_variables));
        if (mem->procesos_p_a_pend > 0)
        {
            prioridad_max_procesos = PAGOS_ANULACIONES;
        }else if (mem->procesos_a_r_pend > 0)
        {
            prioridad_max_procesos = ADMINISTRACION_RESERVAS;
        }
    
        sem_post(&(mem->sem_aux_variables));

        for (int i = 0; i < n_nodos; i++) {
            //Enviamos un mensaje a todos los nodos diciendo que queremos entrar en la sección crítica
            if (id_nodos[i] != mi_id){
                msg_ticket.mtype = id_nodos[i];
                msg_ticket.id_origen = mi_id;
                msg_ticket.ticket_origen = mi_ticket;
                msg_ticket.prioridad = prioridad_max_procesos; //Establecemos el mensaje de envio como la prioridad maxima entre nuestros procesos
                switch (prioridad_max_procesos)
                {
                case ADMINISTRACION_RESERVAS:
                    ack_enviados_a_r++;
                    break;
                case PAGOS_ANULACIONES:
                    ack_enviados_p_a++;
                    break;
                default:
                    break;
                }
                msgsnd(msg_tickets_id, &msg_ticket, sizeof(mensaje), 0); //Enviamos el ticket al nodo 
                

                #ifdef __PRINT_RECIBIR
                printf("Enviando el mensaje %i al nodo %li desde el nodo %li\n",msg_ticket.ticket_origen,msg_ticket.mtype,mi_id);
                #endif // DEBUG
            }
        }
        
        while (1){
            ///SECCIÓN CRÍTICA;


            // FIN DE LA SECCIÓN CRÍTICA
            sem_wait(&(mem->sem_sync_end));// Esperamos a que termine la sección crítica

            printf("Comprobando prioridades\n");
            
            sem_wait(&sem_mutex); // Para no enviar nada hasta enviar todos los acks
            if (prioridad_max_procesos < prioridad_max_recivida_nodos){
                enviar_acks();
                max_intentos = N_MAX_INTENTOS; 
                break; // Salimos del bucle
            }else if(prioridad_max_procesos == prioridad_max_recivida_nodos){
                if (max_intentos == 0)
                {
                    enviar_acks();
                    max_intentos = N_MAX_INTENTOS; // Reiniciamos el contador
                }else {
                    max_intentos --;
                }
                break; // Salimos del bucle
            }else {
                
            }
            sem_post(&sem_mutex);
        }

    }
    return 0;
}

void enviar_acks(){
    if (num_pend_p_a > 0){
        ack(id_nodos_pend_p_a, &num_pend_p_a);
    }else if (num_pend_a_r > 0)
    {
        ack(id_nodos_pend_a_r, &num_pend_a_r);
    }
}

// Funcion para enviar los ack a los distintos nodos de una misma prioridad
void ack(int id_nodos_pend[N-1], int *nodos_pend){
    mensaje msg_tick;
    quiero = 0; 
    for (int i = 0; i < *nodos_pend; i++){
        // Enviamos los mensajes que nos quedasen pendientes de enviar
        msg_tick.mtype = id_nodos_pend[i];
        msg_tick.id_origen = mi_id;
        msg_tick.ticket_origen = ACK;
        msgsnd(msg_tickets_id, &msg_tick, sizeof(mensaje), 0); //Enviamos el mensaje al nodo origen

        #ifdef __PRINT_RECIBIR
            printf("Enviando el ack al nodo %li desde el nodo %li\n",msg_tick.mtype,mi_id);
        #endif // DEBUG
    }
    *nodos_pend = 0;
}






// Creamos la funcion que se encargará de recibir los mensajes
void recibir() {
    mensaje msg_recibir;
    

    #ifdef __PRINT_RECIBIR
    printf("Ejecutando hilo\n");
    #endif 


    while (1) {
        
        msgrcv(msg_tickets_id, &msg_recibir, sizeof(mensaje), mi_id, 0); // Recivimos los mensajes que nos llegan de los nodos

        #ifdef __PRINT_RECIBIR
        printf("Recivimos un mensaje del nodo %i con tipo %li y el ticket es %i\n",msg_recibir.id_origen,msg_recibir.mtype,msg_recibir.ticket_origen);
        #endif 
        

        // Semaforo de exclusión mutua aquí
        sem_wait(&sem_mutex);

        
        // asignamos el valor maximo a ticket maximo
        if (msg_recibir.ticket_origen > max_ticket){ max_ticket = msg_recibir.ticket_origen; }


        if  (
                    (
                        quiero == 0 
                        || msg_recibir.ticket_origen < mi_ticket 
                        || (
                            msg_recibir.ticket_origen == mi_ticket 
                            && msg_recibir.id_origen < mi_id
                            )
                    ) 
                && 
                    (msg_recibir.ticket_origen != ACK)
                && (msg_recibir.prioridad > prioridad_max_procesos )
                    
                    
            )
            {
            // En caso de que no queramos enviar un ticket quiero = 0
            // En caso de que el ticket recivido sea menor que nuestro ticket
            // Si nuestro ticket es igual al recivido pero nuestro id es mayor que el del origen
            // Si la prioridad del proceso es mayor que la nuestra
            

            msg_recibir.mtype = (long) msg_recibir.id_origen;
            msg_recibir.id_origen = (int) mi_id;
            msg_recibir.ticket_origen = ACK; // Si el ticket origen es 0 es que es un ack
            msgsnd(msg_tickets_id, &msg_recibir, sizeof(mensaje), 0); //Enviamos ack al nodo origen

            #ifdef __PRINT_RECIBIR
            printf("Enviamos un mensaje al nodo origen %li\n",msg_recibir.mtype);
            
            #endif // DEBUG



        }else if (msg_recibir.ticket_origen == ACK) // Comprovamos que el ticket no es un ack
        {
            #ifdef __PRINT_RECIBIR
            printf("Hemos recivido un ACK\n");
            printf("pagos y anulaciones %i\n",ack_enviados_p_a);
            printf("administracion y reservas %i\n",ack_enviados_a_r);
            #endif // DEBUG

            // Para entrar en la seccion critica
            switch (msg_recibir.prioridad)
            {
            case PAGOS_ANULACIONES:
                ack_enviados_p_a--;
                if (ack_enviados_p_a == 0)
                {
                    sem_post(&(mem->sem_pagos_anulaciones));
                }
                
                break;
            case ADMINISTRACION_RESERVAS:
                ack_enviados_a_r--;
                if (ack_enviados_a_r == 0)
                {
                    sem_post(&(mem->sem_administracion_reservas));
                }
                break;
            default:
                break;
            }
        }

        else {
            int pri = 0;
            switch (msg_recibir.prioridad)
            {
            case PAGOS_ANULACIONES:
                num_pend_p_a ++;
                id_nodos_pend_p_a[num_pend_p_a] = msg_recibir.id_origen;
                pri = PAGOS_ANULACIONES;
                break;
            case ADMINISTRACION_RESERVAS:
                num_pend_a_r ++;
                id_nodos_pend_a_r[num_pend_a_r] =  msg_recibir.id_origen;
                pri = ADMINISTRACION_RESERVAS;
                break;
            default:
                break;
            }

            if (prioridad_max_recivida_nodos < pri) { prioridad_max_recivida_nodos = pri; } // Actualizamos la prioridad maxima recivida de los nodos
        }
        // Termina el semaforo de exclusion mutua
        sem_post(&sem_mutex);
    }
}

// Funcion para eliminar los buzones cuando hagamos control c
void* fun_ctrl_c(void *args) {
    #ifdef __PRINT_CTRL_C
        printf("Funcion control de terminar el programa funcionando bien\n");
    #endif // DEBUG


    sem_wait(&sem_ctrl_c);

    #ifdef __PRINT_CTRL_C
        printf("\n\n\nEl nodo va ha terminar su ejecución\n");
        printf("Eliminando buzon...\n\n\n\n");
    #endif // DEBUG

    if (msgctl(msg_tickets_id, IPC_RMID, NULL) == -1) {
        perror("Fallo al eliminar el buzon msg_tickets_id con");
        exit(-1);
    }

    #ifdef __PRINT_CTRL_C
        printf("\n\n\nEl nodo va ha terminar su ejecución\n");
        printf("Eliminando memoria compartida...\n\n\n\n");
    #endif // DEBUG

    if (shmctl(memoria_id, IPC_RMID, NULL) == -1){// Eliminamos la zona de memoria compartida
        perror("Fallo al eliminar el buzon msg_tickets_id con");
        exit(-1);
    } 

    
    #ifdef __PRINT_CTRL_C
        printf("\n\nEliminado la memoria y el buzon con exito\n\n");
    #endif // DEBUG
    exit(0);
}


void catch_ctrl_c(int sig)
{
    sem_post(&sem_ctrl_c);
}