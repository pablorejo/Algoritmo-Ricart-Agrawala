#include <pthread.h> // Para hilos
#include <semaphore.h> // Para semáforos
#include "procesos.h" // Incluimos la cabecera de los procesos




int mi_ticket = 0, id_nodos_pend[N-1] = {0}, id_nodos[N-1] = {0}, 
    num_pend = 0, quiero = 0, max_ticket = 0, n_nodos = N-1, ctrl_c = 0;

int msg_tickets_id,msg_semaforo_id,msg_procesos_id; //id del buzón

long mi_id;
int procesos_pendientes = 0;





// Buzones
mensaje msg_ticket;
semaforo msg_semaforo;


sem_t sem_mutex;
sem_t sem_SC;
sem_t sem_ctrl_c;
sem_t sem_proceso_entra;
pthread_t thread_enviar;
pthread_t thread_ctrl_c;


memoria_compartida *mem;

void recibir();
void* enviar(void *args);
void* fun_ctrl_c(void *args);
void catch_ctrl_c(int sig);

int main(int argc, char const *argv[])
{
    if (argc < 3){
        // printf("Introduce el id y cuantos procesos hay\n");
        // exit(-1);


        mi_id = 1; // Guardamos el id que nos otorgara el usuario    
        n_nodos = 2; // Numero de procesos totales
        // Guardando ids de los procesos
        for (int i = 0; i < n_nodos; i++){
            id_nodos[i] = i+1;
        }
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
    
    int permisos = 0666; // permisos de lectura/escritura para todos los usuarios
    int msg_memoria_id = shmget(key+mi_id, sizeof(memoria_compartida), permisos | IPC_CREAT);
    mem = shmat(msg_memoria_id, NULL, 0);

    ///////// Inicializamos la memoria compartida

    // Inicializamos los semaforos de exclusion mutua
    sem_init(&mem->sem_aux_variables,0,1); // Semaforo para leer las variables de la memoria compartida
    sem_init(&mem->sem_mutex,0,1); // Semaforo para entrar en la seccion crítica

    // Semaforos de paso 
    sem_init(&mem->sem_cosultas,0,0);
    sem_init(&mem->sem_reservas,0,0);
    sem_init(&mem->sem_administracion,0,0);


    // Inicializamos las variables a 0
    mem->tenemos_SC = 0;
    mem->procesos_c = 0;
    mem->procesos_a_pend = 0; mem->procesos_c_pend = 0; mem->procesos_r_pend = 0; mem->procesos_p_a_pend = 0;



    // Semaforos de sincronizacion con el proceso recivir
    sem_init(&mem->sem_sync_init,0,0);
    sem_init(&mem->sem_sync_end,0,0);
    sem_init(&mem->sem_sync_intentar,0,0);
    ///////// Fin memoria compartida

    
    #ifdef __PRINT_RECIBIR
    printf("Key 1: %i e id del buzón %i\n",key,msg_tickets_id);

    printf("Key 2: %li e id del buzón %i\n",key+N+mi_id,msg_semaforo_id);
    #endif // DEBUG
 

    msg_ticket.mtype = mi_id;
    msg_ticket.ticket_origen = mi_ticket;

    // iniciamos los semáforos
    sem_init(&sem_mutex,0,1); // Semaforo de exclusión mutua para las variables
    sem_init(&sem_SC,0,0); // Semaforo de paso para el nodo
    sem_init(&sem_ctrl_c,0,0); // Semáforo de paso por si se desea cancelar la ejecucion del nodo
    sem_init(&sem_proceso_entra,0,0);






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

        sem_wait(&mem->sem_sync_intentar); // Esperamos a recivir alguna peticion 
        #ifdef __PRINT_RECIBIR
        printf("Esperando semaforo\n");
        #endif 

        sem_wait(&sem_proceso_entra);

        

        #ifdef __PRINT_RECIBIR
        sleep(SLEEP);
        #endif // DEBUG



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


        for (int i = 0; i < n_nodos; i++) {
            //Enviamos un mensaje a todos los nodos diciendo que queremos entrar en la sección crítica
            if (id_nodos[i] != mi_id){
                msg_ticket.mtype = id_nodos[i];
                msg_ticket.id_origen = mi_id;
                msg_ticket.ticket_origen = mi_ticket;
                msgsnd(msg_tickets_id, &msg_ticket, sizeof(mensaje), 0); //Enviamos el ticket al nodo 


                #ifdef __PRINT_RECIBIR
                printf("Enviando el mensaje %i al nodo %li desde el nodo %li\n",msg_ticket.ticket_origen,msg_ticket.mtype,mi_id);
                #endif // DEBUG
            }
        }
        // printf"Reciviendo mensajes para a todos los nodos\n");

        if (n_nodos > 1){ sem_wait(&sem_SC); } // Comprovamos que no estamos solos para poder entrar en la sección critica
        // El hilo recibir se encargará de sincronizarse con este para entran en la sección crítica
        
        ///SECCIÓN CRÍTICA;
        sem_post(&mem->sem_sync_init);// Avisamos que puede entrar en la sección crítica

        sem_wait(&mem->sem_sync_end);// Esperamos a que termine la sección crítica
        // Fin sección crítica

        
        if (procesos_pendientes > 0)
        {
            break;
        }
        else {
            quiero = 0; 
            for (int i = 0; i < num_pend; i++){
                // Enviamos los mensajes que nos quedasen pendientes de enviar
                msg_ticket.mtype = id_nodos_pend[i];
                msg_ticket.id_origen = mi_id;
                msg_ticket.ticket_origen = ACK;
                msgsnd(msg_tickets_id, &msg_ticket, sizeof(mensaje), 0); //Enviamos el mensaje al nodo origen



                #ifdef __PRINT_RECIBIR
                printf("Enviando el ack al nodo %li desde el nodo %li\n",msg_ticket.mtype,mi_id);
                #endif // DEBUG
            }
            
            num_pend = 0;
        }
    }
    return 0;
}


// Creamos la funcion que se encargará de recibir los mensajes
void recibir() {
    mensaje msg_recibir;
    int ack_recividos = n_nodos;

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
                (msg_recibir.ticket_origen != ACK))
            {
            // En caso de que no queramos enviar un ticket quiero = 0
            // En caso de que el ticket recivido sea menor que nuestro ticket
            // Si nuestro ticket es igual al recivido pero nuestro id es mayor que el del origen
            msg_recibir.mtype = (long) msg_recibir.id_origen;
            msg_recibir.id_origen = (int) mi_id;
            msg_recibir.ticket_origen = ACK; // Si el ticket origen es 0 es que es un ack
            msgsnd(msg_tickets_id, &msg_recibir, sizeof(mensaje), 0); //Enviamos ack al nodo origen


            #ifdef __PRINT_RECIBIR
            printf("Enviamos un mensaje al nodo origen %li\n",msg_recibir.mtype);
            
            #endif // DEBUG



        }else if (msg_recibir.ticket_origen == ACK) // Comprovamos que el ticket no es un ack
        {
            ack_recividos--; 

            #ifdef __PRINT_RECIBIR
            printf("Ack recividos %i\n",ack_recividos);
            #endif // DEBUG


            if (ack_recividos == 1) // Comprobamos que tenemos todos los ack
            {
                sem_post(&sem_SC);  // Indicamos al hilo enviar que puede continuar
                ack_recividos = n_nodos; // Volvemos a actualizar el contador de ack
            }
        }
        else {
            num_pend++;
            id_nodos_pend[num_pend-1] = msg_recibir.id_origen;
        }
        sem_post(&sem_mutex);
        // Termina el semaforo de exclusion mutua
    }
}


void* fun_ctrl_c(void *args) {
    #ifdef __PRINT_CTRL_C
        printf("Funcion control de terminar el programa funcionando bien\n");
    #endif // DEBUG


    sem_wait(&sem_ctrl_c);

    #ifdef __PRINT_CTRL_C
        printf("\n\n\nEl nodo va ha terminar su ejecución\n");
        printf("Eliminando los buzones...\n\n\n\n");
    #endif // DEBUG

    if (msgctl(msg_semaforo_id, IPC_RMID, NULL) == -1) {
        perror("Fallo al eliminar el buzon msg_semaforo_id");
        exit(-1);
    }
    if (msgctl(msg_tickets_id, IPC_RMID, NULL) == -1) {
        perror("Fallo al eliminar el buzon msg_tickets_id con");
        perror("msgctl");
        exit(-1);
    }
    exit(0);
}


void catch_ctrl_c(int sig)
{
    sem_post(&sem_ctrl_c);
}