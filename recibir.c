#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/types.h> // Se definen algunos tipos de datos
#include <sys/ipc.h> // Para algunas banderas
#include <sys/msg.h> // Para usar la funcion


#include <unistd.h>


#include "procesos.h" // Incluimos la cabecera de los procesos




int mi_ticket = 0, id_nodos_pend[N-1] = {0}, id_nodos[N-1] = {0}, 
    num_pend = 0, quiero = 0, max_ticket = 0, n_nodos = N-1;

int msg_tickets_id,msg_ack_id; //id del buzón

long mi_id;

typedef struct 
{
    long mtype ; // Donde guardaremos el nodo origen
    int id_origen;
    int ticket_origen;
}mensaje;
mensaje msg_ticket;
mensaje msg_ack;

sem_t sem_mutex;
pthread_t thread_enviar;


void recibir();
void* enviar(void *args);
semaforo *semaforos;

int main(int argc, char const *argv[])
{
    if (argc < 3){
        // printf("Introduce el id y cuantos procesos hay\n");
        // exit(-1);


        mi_id = 1; // Guardamos el id que nos otorgara el usuario    
        n_nodos = 3; // Numero de procesos totales
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
    printf("Mi id es %li y el N de nodos es %i\n",mi_id,n_nodos);
    key_t key = ftok("recibir.c",213);
    msg_tickets_id = msgget(key,0660 | IPC_CREAT); // Creamos el buzón
    msg_ack_id = msgget(key+1,0660 | IPC_CREAT); // Creamos el buzón

    
    // semaforos_id = shmget(key, sizeof(semaforo), IPC_CREAT | S_IRUSR | S_IWUSR);
    // shmctl(semaforos_id, IPC_RMID, NULL);
    semaforos_id = shmget(key, sizeof(semaforo), IPC_CREAT | 0666 );
    semaforos = (semaforo *)shmat(semaforos_id, NULL, 0);

    msg_ticket.mtype = mi_id;
    msg_ticket.ticket_origen = mi_ticket;

    // iniciamos los semáforos
    sem_init(&sem_mutex,0,1); // Semaforo de exclusión mutua para las variables



    sem_init(&semaforos->sem_mutext,0,1);
    int v_sem_mutex;
    sem_getvalue(&semaforos->sem_mutext,&v_sem_mutex);
    // printf("%i\n",v_sem_mutex);
    sem_init(&semaforos->sem_sync_init,0,0);
    sem_init(&semaforos->sem_sync_end,0,0);
    sem_init((&semaforos->sem_sync_intentar),0,0);



    pthread_create(&thread_enviar, NULL, enviar, NULL);
    
    recibir();
    return 0;
}

void* enviar(void *args) 
{
    printf("Funcion enviar ok\n");
    while (1){
        printf("Esperando semaforo\n");
        int v_sem_mutex;
        sem_getvalue(&semaforos->sem_sync_intentar,&v_sem_mutex);
        printf("%i\n",v_sem_mutex);
        sem_wait(&semaforos->sem_sync_intentar);
        printf("Pulsa enter para entrar en la sección crítica\n");
        sleep(SLEEP);
        printf("Intentando entrar a la sección crítica\n");
        // Semaforo de exclusión mutua aquí
        sem_wait(&sem_mutex);
        quiero = 1;
        mi_ticket = max_ticket + 1;
        sem_post(&sem_mutex);
        // Termina el semaforo
        // printf("Mi ticket es %i\n",msg_ticket.ticket_origen);

        printf("Enviando mensajes para a todos los nodos\n");
        for (int i = 0; i < n_nodos; i++) {
            //Enviamos un mensaje a todos los nodos diciendo que queremos entrar en la sección crítica
            if (id_nodos[i] != mi_id){
                msg_ticket.mtype = id_nodos[i];
                msg_ticket.id_origen = mi_id;
                msg_ticket.ticket_origen = mi_ticket;
                msgsnd(msg_tickets_id, &msg_ticket, sizeof(mensaje), 0); //Enviamos el ticket al nodo 
                printf("Enviando el mensaje %i al nodo %li desde el nodo %li\n",msg_ticket.ticket_origen,msg_ticket.mtype,mi_id);
            }
        }
        // printf"Reciviendo mensajes para a todos los nodos\n");

        // Tenemos que recibir mensajes de todos los nodos para poder ejecutar la sección crítica
        for (int i = 1; i <= n_nodos; i++){
            // Intentaresmo recibir respuesta de todos los nodos y cuando esto pase iremos a nuestra sección crítica
            if (i != mi_id)
            {
                msgrcv(msg_ack_id, &msg_ticket, sizeof(mensaje), mi_id, 0); //Recivimos el mensajes de tipo ack
                
                printf("Recivimos el mensaje de confirmación del nodo %i\n",msg_ticket.id_origen);
            }
        } 
        ///SECCIÓN CRÍTICA;

        // Aquí tenemos que poner un semaforo de sincronizacion con el proceso para que puenda entrar en sección crítica
        sem_post(&semaforos->sem_sync_init);

        // Aquí tenemos que poner un semaforo para que el proceso se sincronice con nosotros y así poder salir de la sección crítica
        sem_wait(&semaforos->sem_sync_end);
        // Fin sección crítica


        sem_post(&semaforos->sem_sync_intentar);

        quiero = 0;

        for (int i = 0; i < num_pend; i++){
            // Enviamos los mensajes que nos quedasen pendientes de enviar
            msg_ticket.mtype = id_nodos_pend[i];
            msg_ticket.id_origen = mi_id;
            msgsnd(msg_ack_id, &msg_ticket, sizeof(mensaje), 0); //Enviamos el mensaje al nodo origen
            // printf"Enviando el ack al nodo %li desde el nodo %li\n",msg_ticket.mtype,mi_id);
        }
        
        num_pend = 0;
    }
}


// Creamos la funcion que se encargará de recibir los mensajes
void recibir() {
    mensaje msg_recibir;
    printf("Ejecutando hilo\n");
    while (1) {
        
        msgrcv(msg_tickets_id, &msg_recibir, sizeof(mensaje), mi_id, 0);
        // printf("Recivimos un mensaje del nodo %i con tipo %li yl el ticket es %i\n",msg_recibir.id_origen,msg_recibir.mtype,msg_recibir.ticket_origen);

        // Semaforo de exclusión mutua aquí
        sem_wait(&sem_mutex);
        // asignamos el valor maximo a ticket maximo
        if (msg_recibir.ticket_origen > max_ticket){ max_ticket = msg_recibir.ticket_origen; }

        if ((quiero == 0 || msg_recibir.ticket_origen < mi_ticket || (msg_recibir.ticket_origen == mi_ticket && (msg_recibir.id_origen < mi_id)))){
            // En caso de que no queramos enviar un ticket quiero = 0
            // En caso de que el ticket recivido sea menor que nuestro ticket
            // Si nuestro ticket es igual al recivido pero nuestro id es mayor que el del origen
            msg_recibir.mtype = (long) msg_recibir.id_origen;
            msg_recibir.id_origen = (int) mi_id;
            msgsnd(msg_ack_id, &msg_recibir, sizeof(mensaje), 0); //Enviamos ack al nodo origen
            // printf("Enviamos un mensaje al nodo origen %li\n",msg_recibir.mtype);
        }
        else {
            num_pend++;
            id_nodos_pend[num_pend-1] = msg_recibir.id_origen;
        }
        sem_post(&sem_mutex);
        // Termina el semaforo de exclusion mutua
    }
}
