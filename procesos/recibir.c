#include <pthread.h> // Para hilos
#include <semaphore.h> // Para semáforos
#include "procesos.h" // Incluimos la cabecera de los procesos




int  ctrl_c = 0;

// Colas de pendientes de las prioridades recibidas de otros nodos
int num_pend_p_a = 0,num_pend_a_r = 0;








int max_intentos = N_MAX_INTENTOS;



sem_t sem_ctrl_c; // semaforo de paso para realizar el control c
pthread_t thread_enviar; 
pthread_t thread_ctrl_c;



void recibir();
void* fun_ctrl_c(void *args);


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
            
        #endif // DEBUG

    }else{
        mi_id = atoi(argv[1]); // Guardamos el id que nos otorgara el usuario    
        n_nodos = atoi(argv[2]); // Numero de nodos totales
        
        
    }
  

    #ifdef __PRINT_RECIBIR
    printf("Mi id es %li y el numero de nodos es %i\n",mi_id,n_nodos);
    #endif // DEBUG



    key_t key = ftok(CARPETA,1);

    msg_tickets_id = msgget(key, 0660 | IPC_CREAT); // Creamos el buzón


    // Memoria compartida
    

    memoria_id = shmget(key+mi_id, sizeof(memoria_compartida), 0660 | IPC_CREAT);
    if (memoria_id == -1){
        perror("Error al intentar crear la memoria compartida");
    }
    mem = shmat(memoria_id, NULL, 0);


    
    ///////// Inicializamos la memoria compartida
    mem->quiero = 0;
    mem->mi_id = mi_id; mem->n_nodos = n_nodos;
    mem->max_ticket = 0; mem->mi_ticket = 1;

    for (int i = 0; i < n_nodos; i++){
        mem->id_nodos[i] = i+1;
    }

    // Arrays de los id pendientes
    // mem->id_nodos_pend_pagos_anulaciones = {0};
    // mem->id_nodos_pend_administracion_reservas = {0};
    // mem->id_nodos_pend_consultas = {0};
    // mem->tickets_pend_pagos_anulaciones = {0}

    //Intra nodo
    mem->pend_pagos_anulaciones = 0; mem->pend_administracion_reservas = 0; mem->pend_consultas = 0;
    mem->prioridad_max_enviada = 0;
    sem_init(&(mem->sem_mutex),1,1); // Semaforo de exclusion mutua intra nodos

    mem->n_consultas = 0; mem->esperando_consultas = 0; mem->esperando = 0;
    sem_init(&(mem->sem_pro_n_consultas),1,1);// Semaforo para protejer la variable conpartida de n_consultas
    sem_init(&(mem->sem_ctrl_paso_consultas),1,0);

    //Entre nodos
    mem->nodos_pend_administracion_reservas = 0; mem->nodos_pend_pagos_anulaciones = 0; mem->nodos_pend_consultas = 0;
    mem->ack_pend_administracion_reservas = 0; mem->ack_pend_pagos_anulaciones = 0; mem->ack_pend_consultas = 0;
    mem->intentos = N_MAX_INTENTOS;

    // Semaforos de paso 
    sem_init(&(mem->sem_paso_pagos_anulaciones),1,0);
    sem_init(&(mem->sem_paso_administracion_reservas),1,0);
    sem_init(&(mem->sem_paso_consultas),1,0);
    // sem_init(&(mem->sem_sync_siguiente),1,0);



    // Proteccion de memoria compartida
    sem_init(&(mem->sem_aux_variables),1,1);


    // Para recabar datos
    #ifdef __RECABAR_DATOS
        sem_init(&(mem->sem_elapse_pagos_anulaciones),1,1); sem_init(&(mem->sem_elapse_administracion_reservas),1,1); sem_init(&(mem->sem_elapse_consultas),1,1);
        mem->num_elapse_pagos_anulaciones = 0; mem->num_elapse_administracion_reservas = 0; mem->num_elapse_consultas = 0;
    #endif // DEBUG



    #ifdef __PRINT_RECIBIR
    printf("Key 1: %i e id del buzón %i\nID de la memoria compartida es %i\n",key,msg_tickets_id,memoria_id);

    #endif // DEBUG
 


    // iniciamos los semáforos
    sem_init(&sem_ctrl_c,1,0); // Semáforo de paso por si se desea cancelar la ejecucmax_ticket



    mem->tenemos_SC = 0;
    pthread_create(&thread_ctrl_c, NULL, fun_ctrl_c, NULL);
    // Controlar el ctrl+c
    signal(SIGINT, &catch_ctrl_c);

    recibir();
    return 0;
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
        

        #ifdef __PRINT_RECIBIR
        // printf("mem->quiero = %i\n",mem->quiero);
        // printf("msg_recibir.ticket_origen = %i\n",msg_recibir.ticket_origen);
        // printf("mem->mi_ticket = %i\n",mem->mi_ticket);
        // printf("msg_recibir.id_origen  = %i\n",msg_recibir.id_origen);

        // printf("mem->mi_id = %li\n",mem->mi_id);
        // printf("msg_recibir.prioridad = %i\n",msg_recibir.prioridad);

        // printf("mem->prioridad_max_enviada = %i\n",mem->prioridad_max_enviada);
        // printf("mem->n_consultas = %i\n",mem->n_consultas);
        // printf("mem->tenemos_SC = %i\n",mem->tenemos_SC);
        #endif 

        if  (
                    (
                           mem->quiero == 0 
                        || msg_recibir.ticket_origen < mem->mi_ticket 
                        || (
                            msg_recibir.ticket_origen == mem->mi_ticket 
                            && msg_recibir.id_origen < mem->mi_id
                            )
                        || msg_recibir.prioridad > mem->prioridad_max_enviada // En el caso de que la prioridad recivida sea mayor que la prioridad maxima nuestra enviada
                        // || mem->n_consultas > 0 // Comprobamos si se estan ejecutando consultas
                        || (mem->prioridad_max_enviada == msg_recibir.prioridad && mem->prioridad_max_enviada == CONSULTAS)
                    )
                &&
                    (msg_recibir.ticket_origen != ACK)
                && (msg_recibir.prioridad >= mem->prioridad_max_enviada)
                && (mem->tenemos_SC == 0 || (mem->prioridad_max_enviada == msg_recibir.prioridad && mem->prioridad_max_enviada == CONSULTAS)) // Aqui comprovamos que no tenemos la SC es decir que no hemos recivido todos los acks pendientes o bien que estamos ejecutando consultas
            )
            {
            // En caso de que no queramos enviar un ticket quiero = 0
            // En caso de que el ticket recivido sea menor que nuestro ticket
            // Si nuestro ticket es igual al recivido pero nuestro id es mayor que el del origen
            // Si la prioridad del proceso es mayor que la nuestra
            

            // Comprobamos si la prioridad es mayor a la que tienen nuestro procesos en ese caso tendremos que volver a mandar las peticiones unicamente al nodo que nos mandó la prioridad

            if (msg_recibir.prioridad > mem->prioridad_max_enviada && mem->quiero == 1){
                mensaje msg;
                if (mem->prioridad_max_enviada == ADMINISTRACION_RESERVAS){
                    mem->ack_pend_administracion_reservas ++; //Aumentamos el numero de acks pendientes de ser atendidos
                }else{
                    mem->ack_pend_consultas ++;
                }
                msg.mtype = msg_recibir.id_origen; // Lo enviamos a la id origen del mensaje recivido
                msg.id_origen = mem->mi_id; // Lo mandamos desde nuestra id
                msg.prioridad = mem->prioridad_max_enviada; // Lo mandamos con la prioridad maxima actual de nuestro nodo
                msgsnd(msg_tickets_id, &msg, sizeof(mensaje), 0); //Enviamos ack al nodo origen
                #ifdef __PRINT_RECIBIR 
                    printf("Hemos enviado una peticion de ack puesto que nuestra prioridad es mas pequeña que esta\n");
                #endif // DEBUG
            }

            // Enviamos el ack de confirmacion
            msg_recibir.mtype = (long) msg_recibir.id_origen;
            msg_recibir.id_origen = (int) mem->mi_id;
            msg_recibir.ticket_origen = ACK; // Si el ticket origen es 0 es que es un ack
            msgsnd(msg_tickets_id, &msg_recibir, sizeof(mensaje), 0); //Enviamos ack al nodo origen

            #ifdef __PRINT_RECIBIR
                printf("Enviamos un mensaje de tipo ACK al nodo origen %li desde el hilo recibir\n",msg_recibir.mtype);
            #endif // DEBUG


        // Aqui comprobaremos los acks
        }else if (msg_recibir.ticket_origen == ACK) // Comprovamos que el ticket no es un ack
        {
            #ifdef __PRINT_RECIBIR
                printf("Hemos recivido un ACK\n");
                printf("pagos y anulaciones %i\n",mem->ack_pend_pagos_anulaciones);
                printf("administracion y reservas %i\n",mem->ack_pend_administracion_reservas);
                printf("Consultas %i\n",mem->ack_pend_consultas);
                printf("prioridad del ack %i\n",msg_recibir.prioridad);
            #endif // DEBUG

            // Para entrar en la seccion critica

            
            switch (msg_recibir.prioridad)
            {
            case PAGOS_ANULACIONES:
                mem->ack_pend_pagos_anulaciones--;

                #ifdef __PRINT_RECIBIR
                    printf("PAGOS_ANULACIONES\n");
                    printf("ACK pendientes de pagos o anulaiones %i\n",mem->ack_pend_pagos_anulaciones);
                #endif

                if (mem->ack_pend_pagos_anulaciones == 0 && mem->tenemos_SC == 0 && mem->pend_pagos_anulaciones > 0)
                {
                    mem->tenemos_SC = 1;
                    mem->intentos = N_MAX_INTENTOS;
                    sem_post(&(mem->sem_paso_pagos_anulaciones));
                    #ifdef __PRINT_RECIBIR
                        printf("Dejamos que el proceso de pagos o anulaciones pueda entrar\n");
                    #endif
                }
                
                break;
            case ADMINISTRACION_RESERVAS:


                mem->ack_pend_administracion_reservas--;

              
                #ifdef __PRINT_RECIBIR
                    printf("ADMINISTRACION_RESERVAS\n");
                    printf("ACK pendientes de adminsitracion o reservas %i\n",mem->ack_pend_administracion_reservas);
                #endif
                if (mem->ack_pend_administracion_reservas == 0 && mem->tenemos_SC == 0 && mem->pend_administracion_reservas > 0)
                {
                    if (mem->pend_pagos_anulaciones == 0)
                    {
                        mem->tenemos_SC = 1;
                        mem->intentos = N_MAX_INTENTOS;
                        sem_post(&(mem->sem_paso_administracion_reservas));
                        #ifdef __PRINT_RECIBIR
                            printf("Dejamos que el proceso de administración o reservas pueda entrar\n");
                        #endif
                    }else{
                        enviar_tickets(ADMINISTRACION_RESERVAS);
                    }
                }
                
                break;
            case CONSULTAS:


                mem->ack_pend_consultas--;


                #ifdef __PRINT_RECIBIR
                    printf("CONSULTAS\n");
                    printf("ACK pendientes de consultas %i\n",mem->ack_pend_consultas);
                #endif

                if (mem->ack_pend_consultas == 0 && mem->tenemos_SC == 0 && mem->pend_consultas > 0)
                {
                    if (mem->pend_pagos_anulaciones == 0 && mem->pend_administracion_reservas == 0)
                    {
                        mem->tenemos_SC = 1;
                        mem->intentos = N_MAX_INTENTOS;
                        sem_post(&(mem->sem_paso_consultas));
                        #ifdef __PRINT_RECIBIR
                            printf("Dejamos que el proceso de consultas pueda entrar\n");
                        #endif
                    }else{
                        enviar_tickets(CONSULTAS);
                    }
                }
            
                
                break;
            default:
                break;
            }
        }

        else { // En caso de que no sea un ack ni tampoco se pueda enviar un ack entonces se pondran en cola
            #ifdef __PRINT_RECIBIR
                printf("Recivimos proceso pendiente con prioridad %i\n",msg_recibir.prioridad);
            #endif // DEBUG
            switch (msg_recibir.prioridad)
            {
            case PAGOS_ANULACIONES:
                // mem->ack_numero_1_consultas = 0;
                // mem->ack_numero_1_administracio_reservas = 0;
                mem->tickets_pend_pagos_anulaciones[mem->nodos_pend_pagos_anulaciones] = msg_recibir.ticket_origen;
                mem->id_nodos_pend_pagos_anulaciones[mem->nodos_pend_pagos_anulaciones] = msg_recibir.id_origen;
                mem->nodos_pend_pagos_anulaciones ++;
                break;
            case ADMINISTRACION_RESERVAS:
                // mem->ack_numero_1_consultas = 0;
                mem->tickets_pend_administracion_reservas[mem->nodos_pend_administracion_reservas] = msg_recibir.ticket_origen;
                mem->id_nodos_pend_administracion_reservas[mem->nodos_pend_administracion_reservas] =  msg_recibir.id_origen;
                mem->nodos_pend_administracion_reservas ++;
                break;
            case CONSULTAS:
                mem->tickets_pend_consultas[mem->nodos_pend_consultas] = msg_recibir.ticket_origen;
                mem->id_nodos_pend_consultas[mem->nodos_pend_consultas] = msg_recibir.id_origen;
                mem->nodos_pend_consultas++;
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


    #ifdef __RECABAR_DATOS
    // Aqui es donde guardaremos los ficheros de datos para cada nodo
        sem_wait(&(mem->sem_elapse_pagos_anulaciones));

        char nombre_archivo[100];
        sprintf(nombre_archivo, "datos_nodo_%ld_pagos_anulacioens.txt", mem->mi_id); 
        FILE *archivo_pagos;
        archivo_pagos = fopen(nombre_archivo, "w");

        if (archivo_pagos == NULL) { // Verificar que el archivo se abrió correctamente
            printf("No se pudo abrir el archivo.\n");
            return 0;
        }

        for (int i = 0; i < mem->num_elapse_pagos_anulaciones; i++) {
            fprintf(archivo_pagos, "%f\n", mem->elapse_time_pagos_anulaciones[i]);
        }
        fclose(archivo_pagos); // Cerrar el archivo

        sem_post(&(mem->sem_elapse_pagos_anulaciones));




        sem_wait(&(mem->sem_elapse_administracion_reservas));

        sprintf(nombre_archivo, "datos_nodo_%ld_administracion_reservas.txt", mem->mi_id); 
        FILE *archivo_reservas;
        archivo_reservas = fopen(nombre_archivo, "w");

        if (archivo_reservas == NULL) { // Verificar que el archivo se abrió correctamente
            printf("No se pudo abrir el archivo.\n");
            return 0;
        }

        for (int i = 0; i < mem->num_elapse_administracion_reservas; i++) {
            fprintf(archivo_reservas, "%f\n", mem->elapse_time_administracion_reservas[i]);
        }
        fclose(archivo_reservas); // Cerrar el archivo

        sem_post(&(mem->sem_elapse_administracion_reservas));




        sem_wait(&(mem->sem_elapse_consultas));

        sprintf(nombre_archivo, "datos_nodo_%ld_consultas.txt", mem->mi_id); 
        FILE *archivo_consultas;
        archivo_consultas = fopen(nombre_archivo, "w");

        if (archivo_consultas == NULL) { // Verificar que el archivo se abrió correctamente
            printf("No se pudo abrir el archivo.\n");
            return 0;
        }

        for (int i = 0; i < mem->num_elapse_consultas; i++) {
            fprintf(archivo_consultas, "%f\n", mem->elapse_time_consultas[i]);
        }
        fclose(archivo_consultas); // Cerrar el archivo

        sem_post(&(mem->sem_elapse_consultas));

        #ifdef __PRINT_CTRL_C
        printf("\n\nFin de la funcion CTRL+C\n\n\n");
        #endif // DEBUG

    #endif // DEBUG



    #ifdef __PRINT_CTRL_C
        printf("\n\n\nEl nodo va ha terminar su ejecución\n");
        printf("Eliminando buzon...\n\n\n\n");

    if (msgctl(msg_tickets_id, IPC_RMID, NULL) == -1) {
        perror("Fallo al eliminar el buzon msg_tickets_id con");
        exit(-1);
    }
    #endif // DEBUG

    #ifdef __PRINT_CTRL_C
        printf("\n\n\nEl nodo va ha terminar su ejecución\n");
        printf("Eliminando memoria compartida...\n\n\n\n");


    if (shmctl(memoria_id, IPC_RMID, NULL) == -1){// Eliminamos la zona de memoria compartida
        perror("Fallo al eliminar el buzon msg_tickets_id con");
        exit(-1);
    } 
    #endif // DEBUG

    
    
    #ifdef __PRINT_CTRL_C
        printf("\n\nEliminado la memoria y el buzon con exito\n\n");
    #endif // DEBUG
    exit(0);
}


void catch_ctrl_c(int sig)
{
    sem_post(&sem_ctrl_c);
}
