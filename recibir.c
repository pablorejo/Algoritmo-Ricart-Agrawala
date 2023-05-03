#include <pthread.h> // Para hilos
#include <semaphore.h> // Para semáforos
#include "procesos.h" // Incluimos la cabecera de los procesos




int  ctrl_c = 0;

// Colas de pendientes de las prioridades recibidas de otros nodos
int num_pend_p_a = 0,num_pend_a_r = 0;
int id_nodos_pend_pagos_anulaciones[N-1] = {0}, id_nodos_pend_administracion_reservas[N-1] = {0}, id_nodos_pend_consultas;




int msg_tickets_id; //id del buzón




int max_intentos = N_MAX_INTENTOS;

// Buzones
mensaje msg_ticket;
semaforo msg_semaforo;


sem_t sem_ctrl_c; // semaforo de paso para realizar el control c
pthread_t thread_enviar; 
pthread_t thread_ctrl_c;

int memoria_id;
memoria_compartida *mem;

void siguiente();
void* enviar(void *args);
void* fun_ctrl_c(void *args);
void catch_ctrl_c(int sig);
void ack(int id_nodos_pend[N-1], int *nodos_pend, int prioridade);
void enviar_acks();
void tenemosSC();
void NoTenemosSC();

int main(int argc, char const *argv[])
{
    long mi_id;
    int n_nodos;
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
    printf("Mi id es %li y el numero de nodos es %i\n",mi_id,n_nodos);
    #endif // DEBUG



    key_t key = ftok(".",1);

    msg_tickets_id = msgget(key,0660 | IPC_CREAT); // Creamos el buzón


    // Memoria compartida
    

    memoria_id = shmget(key+mi_id, sizeof(memoria_compartida), 0666 | IPC_CREAT);
    mem = shmat(memoria_id, NULL, 0);
    ///////// Inicializamos la memoria compartida
    mem->quiero = 0;
    mem->mi_id = mi_id; mem->n_nodos = n_nodos;
    mem->max_ticket = 0; mem->mi_ticket = 1;

    //Intra nodo
    mem->pend_pagos_anulaciones = 0; mem->pend_administracion_reservas = 0; mem->pend_consultas = 0;
    mem->prioridad_max_enviada = 0;


    //Entre nodos
    mem->nodos_pend_administracion_reservas = 0; mem->nodos_pend_pagos_anulaciones = 0; mem->nodos_pend_consultas = 0;
    mem->ack_pend_administracion_reservas = 0; mem->ack_pend_pagos_anulaciones = 0; mem->ack_pend_consultas = 0;
    mem->intentos = N_MAX_INTENTOS;
    // Inicializamos los semaforos de exclusion mutua


    // Semaforos de paso 
    sem_init(&(mem->sem_paso_pagos_anulaciones),1,0);
    sem_init(&(mem->sem_paso_administracion_reservas),1,0);
    sem_init(&(mem->sem_paso_consultas),1,0);
    // sem_init(&(mem->sem_sync_enviar_ack),1,0);
    sem_init(&(mem->sem_sync_siguiente),1,0);



    // Proteccion de memoria compartida
    sem_init(&(mem->sem_pro_ack,1,1));
    sem_init(&(mem->sem_pro_pend,1,1));
    
    #ifdef __PRINT_RECIBIR
    printf("Key 1: %i e id del buzón %i\nID de la memoria compartida es %i\n",key,msg_tickets_id,memoria_id);

    #endif // DEBUG
 

    msg_ticket.mtype = mem->mi_id;
    msg_ticket.ticket_origen = mem->mi_ticket;

    // iniciamos los semáforos
    sem_init(&(mem->sem_aux_variables),1,1); // Semaforo de exclusión mutua para las variables
    sem_init(&sem_ctrl_c,1,0); // Semáforo de paso por si se desea cancelar la ejecucmax_ticket




    NoTenemosSC();
    pthread_create(&thread_enviar, NULL, enviar, NULL);
    pthread_create(&thread_ctrl_c, NULL, fun_ctrl_c, NULL);
    // Controlar el ctrl+c
    signal(SIGINT, &catch_ctrl_c);

    siguiente();
    return 0;
}


void siguiente(){
    sem_wait(&(mem->sem_sync_siguiente));
    if (mem->pend_pagos_anulaciones > 0){
        if (mem->nodos_pend_pagos_anulaciones > 0 && mem->intentos = 0){
            mem->tenemos_SC = 0;
            enviar_ack(); // No dejamos pasar a mas procesos de pagos y hacemos que se envien los ack
        }else {
            mem->intentos --;
            sem_post(&(mem->sem_paso_pagos_anulaciones)); // Dejamos pasar a otro proceso de pagos
        }
    }else if (mem->nodos_pend_pagos_anulaciones > 0){
        mem->tenemos_SC = 0;
        enviar_ack(); // No dejamos pasar a mas procesos;
    }else if (mem->pend_administracion_reservas > 0){
        if (mem->nodos_pend_administracion_reservas > 0 && mem->intentos = 0){
            mem->tenemos_SC = 0;
            enviar_ack(); // No dejamos pasar a mas procesos de pagos y hacemos que se envien los ack
        }else {
            mem->intentos --;
            sem_post(&(mem->sem_paso_administracion_reservas)); // Dejamos pasar a otro proceso de pagos
        }
    }else if (mem->nodos_pend_administracion_reservas > 0){
        mem->tenemos_SC = 0;
        enviar_ack();
    }else{
        if (mem->pend_consultas > 0){
            sem_post(&(mem->sem_paso_consultas));
        }
        if (mem->nodos_pend_consultas>0){
            enviar_ack();
        }
    }
}

void enviar_acks(){
    // Semaforo de paso
    // sem_wait(&(mem->sem_sync_enviar_ack));


    // Si quiero = 0 enviamos ack a todos los procesos
    if (mem->quiero == 0){
        ack(id_nodos_pend_pagos_anulaciones, &mem->nodos_pend_pagos_anulaciones, PAGOS_ANULACIONES);
        ack(id_nodos_pend_administracion_reservas, &mem->nodos_pend_administracion_reservas, ADMINISTRACION_RESERVAS);
        ack(id_nodos_pend_consultas, &mem->nodos_pend_consultas, CONSULTAS);
    }else{
        if (mem->nodos_pend_pagos_anulaciones > 0){
            ack(id_nodos_pend_pagos_anulaciones, &mem->nodos_pend_pagos_anulaciones, PAGOS_ANULACIONES);
        }else if(mem->nodos_pend_administracion_reservas > 0){
            ack(id_nodos_pend_administracion_reservas, &mem->nodos_pend_administracion_reservas, ADMINISTRACION_RESERVAS);
        }else if(mem->nodos_pend_consultas > 0){
            ack(id_nodos_pend_consultas, &mem->nodos_pend_consultas, CONSULTAS);
        }
    }
}

// Funcion para enviar los ack a los distintos nodos de una misma prioridad
void ack(int id_nodos_pend[N-1], int *nodos_pend, int prioridade){
    mensaje msg_tick;
    for (int i = 0; i < *nodos_pend; i++){
        // Enviamos los mensajes que nos quedasen pendientes de enviar
        msg_tick.mtype = id_nodos_pend[i];
        msg_tick.id_origen = mem->mi_id;
        msg_tick.ticket_origen = ACK;
        msg_tick.prioridad = prioridade;
        msgsnd(msg_tickets_id, &msg_tick, sizeof(mensaje), 0); //Enviamos el mensaje al nodo origen

        #ifdef __PRINT_RECIBIR
            printf("Enviando el ack al nodo %li desde el nodo %li con prioridad %i\n",msg_tick.mtype,mem->mi_id,prioridade);
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
        
        msgrcv(msg_tickets_id, &msg_recibir, sizeof(mensaje), mem->mi_id, 0); // Recivimos los mensajes que nos llegan de los nodos

        #ifdef __PRINT_RECIBIR
        printf("Recivimos un mensaje del nodo %i con tipo %li y el ticket es %i con prioridad %i\n",msg_recibir.id_origen,msg_recibir.mtype,msg_recibir.ticket_origen,msg_recibir.prioridad);
        #endif 
        

        // Semaforo de exclusión mutua aquí
        sem_wait(&(mem->sem_aux_variables));

        
        // asignamos el valor maximo a ticket maximo
        if (msg_recibir.ticket_origen > mem->max_ticket){ mem->max_ticket = msg_recibir.ticket_origen; }



        // Comprobamos la prioridad maxima de los procesos en este momento
        int prioridad_max_procesos;
        if (mem->pend_pagos_anulaciones > 0){
            prioridad_max_procesos = PAGOS_ANULACIONES;
        }else if (mem->pend_administracion_reservas > 0){
            prioridad_max_procesos = ADMINISTRACION_RESERVAS;
        }else {
            prioridad_max_procesos = CONSULTAS;
        }
        

        if  (
                    (
                           mem->quiero == 0 
                        || msg_recibir.ticket_origen < mem->mi_ticket 
                        || (
                            msg_recibir.ticket_origen == mem->mi_ticket 
                            && msg_recibir.id_origen < mem->mi_id
                            )
                    ) 
                && 
                    (msg_recibir.ticket_origen != ACK)
                && (msg_recibir.prioridad >= prioridad_max_procesos )
                && (mem->tenemos_SC == 0) // Aqui comprovamos que no tenemos la SC es decir que no hemos recivido todos los acks pendientes
            )
            {
            // En caso de que no queramos enviar un ticket quiero = 0
            // En caso de que el ticket recivido sea menor que nuestro ticket
            // Si nuestro ticket es igual al recivido pero nuestro id es mayor que el del origen
            // Si la prioridad del proceso es mayor que la nuestra
            

            // Comprobamos si la prioridad es mayor a la que tienen nuestro procesos en ese caso tendremos que volver a mandar las peticiones unicamente al nodo que nos mandó la prioridad
            if (msg_recibir.prioridad > prioridad_max_procesos){
                mensaje msg;
                mem->ack_pend_administracion_reservas ++; //Aumentamos el numero de acks pendientes de ser atendidos
                msg.mtype = msg_recibir.id_origen; // Lo enviamos a la id origen del mensaje recivido
                msg.id_origen = mem->mi_id; // Lo mandamos desde nuestra id
                msg.prioridad = prioridad_max_procesos; // Lo mandamos con la prioridad maxima actual de nuestro nodo
            }

            // Enviamos el ack de confirmacion
            msg_recibir.mtype = (long) msg_recibir.id_origen;
            msg_recibir.id_origen = (int) mem->mi_id;
            msg_recibir.ticket_origen = ACK; // Si el ticket origen es 0 es que es un ack
            msgsnd(msg_tickets_id, &msg_recibir, sizeof(mensaje), 0); //Enviamos ack al nodo origen

            #ifdef __PRINT_RECIBIR
            printf("Enviamos un mensaje al nodo origen %li\n",msg_recibir.mtype);
            
            #endif // DEBUG


        // Aqui comprobaremos los acks
        }else if (msg_recibir.ticket_origen == ACK) // Comprovamos que el ticket no es un ack
        {
            #ifdef __PRINT_RECIBIR
            printf("Hemos recivido un ACK\n");
            printf("pagos y anulaciones %i\n",mem->ack_enviados_p_a);
            printf("administracion y reservas %i\n",mem->ack_enviados_a_r);
            printf("prioridad del ack %i\n",msg_recibir.prioridad);
            #endif // DEBUG

            // Para entrar en la seccion critica


            switch (msg_recibir.prioridad)
            {
            case PAGOS_ANULACIONES:
                printf("pagos\n");
                mem->ack_pend_pagos_anulaciones--;
                if (mem->ack_pend_pagos_anulaciones == 0)
                {
                    tenemosSC();
                    sem_post(&(mem->sem_paso_pagos_anulaciones));
                    printf("Dejamos que el proceso de pagos o anulaciones pueda entrar\n");
                }
                
                break;
            case ADMINISTRACION_RESERVAS:
                printf("ADMINISTRACION_RESERVAS\n");

                mem->ack_pend_administracion_reservas--;
                if (mem->ack_pend_administracion_reservas == 1)
                {
                    tenemosSC();
                    sem_post(&(mem->sem_paso_administracion_reservas));
                    printf("Dejamos que el proceso de administración o reservas pueda entrar\n");
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
                id_nodos_pend_pagos_anulaciones[mem->pend_pagos_anulaciones] = msg_recibir.id_origen;
                mem->pend_pagos_anulaciones ++;
                pri = PAGOS_ANULACIONES;
                break;
            case ADMINISTRACION_RESERVAS:
                id_nodos_pend_administracion_reservas[mem->pend_pagos_anulaciones] =  msg_recibir.id_origen;
                mem->pend_pagos_anulaciones ++;
                pri = ADMINISTRACION_RESERVAS;
                break;
            case CONSULTAS:
                id_nodos_pend_consultas[mem->pend_consultas] = msg_recibir.id_origen;
                mem->pend_consultas++;
                pri = CONSULTAS;
            default:
                break;
            }
        }
        // Termina el semaforo de exclusion mutua
        sem_post(&(mem->sem_aux_variables));
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

void tenemosSC(){
    mem->tenemos_SC = 1;
}

void NoTenemosSC(){
    mem->tenemos_SC = 0;
}

