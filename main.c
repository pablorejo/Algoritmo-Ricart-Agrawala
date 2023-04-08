#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/types.h> // Se definen algunos tipos de datos
#include <sys/ipc.h> // Para algunas banderas
#include <sys/msg.h> // Para usar la funcion

#include <semaphore.h>
#define N 1000

int mi_ticket = 0, id_nodos_pend[N-1] = {0}, num_pend = 0, quiero = 0, max_ticket = 0, n_procesos = N-1;
int msg_tickets,msg_ack; //id del buzón

int id_nodos[N-1] = {0};//Id de todos los nodos
long mi_id;

typedef struct 
{
    long mtype ; // Donde guardaremos el nodo origen
    int id_origen;
    int ticket_origen;
}mensaje;


sem_t mutex;
pthread_t mi_id_thread;


void* recivir(void *args);

int main(int argc, char const *argv[])
{
    if (argc < 3){
        // printf("Introduce el id y cuantos procesos hay\n");
        // exit(-1);
        mi_id = 1; // Guardamos el id que nos otorgara el usuario    
        n_procesos = 3; // Numero de procesos totales

        // Guardando ids de los procesos
        for (int i = 0; i < n_procesos; i++){
            id_nodos[i] = i+1;
        }
    }else{
        mi_id = atoi(argv[1]); // Guardamos el id que nos otorgara el usuario    
        n_procesos = atoi(argv[2]); // Numero de procesos totales

        // Guardando ids de los procesos
        for (int i = 0; i < n_procesos; i++){
            id_nodos[i] = i+1;
        }
    }
    printf("Mi id es %li y el N de procesos es %i\n",mi_id,n_procesos);
    key_t key = ftok("main.c",2);
    msg_tickets = msgget(key,0660 | IPC_CREAT); // Creamos el buzón
    msg_ack = msgget(key+1,0660 | IPC_CREAT); // Creamos el buzón


    mensaje msg;
    msg.mtype = mi_id;
    msg.ticket_origen = 0;

    // iniciamos el semáforo
    sem_init(&mutex,0,1);


    pthread_create(&mi_id_thread, NULL, recivir, NULL);
    
    while (1){
        printf("Pulsa enter para entrar en la sección crítica\n");
        while (getchar()!='\n'){}
        printf("Intentando entrar a la sección crítica\n");
        // Semaforo de exclusión mutua aquí
        sem_wait(&mutex);
        quiero = 1;
        msg.ticket_origen = max_ticket + 1;
        sem_post(&mutex);
        // Termina el semaforo

        printf("Enviando mensajes para a todos los nodos\n");
        for (int i = 0; i < n_procesos; i++) {
            //Enviamos un mensaje a todos los nodos diciendo que queremos entrar en la sección crítica
            if (id_nodos[i] != mi_id){
                msg.mtype = id_nodos[i];
                msg.id_origen = mi_id;
                msgsnd(msg_tickets, &msg, sizeof(mensaje), 0); //Enviamos el mensaje al nodo origen
                printf("Enviando el mensaje %i al nodo %li desde el nodo %li\n",msg.ticket_origen,msg.mtype,mi_id);
            }
        }
        printf("Reciviendo mensajes para a todos los nodos\n");

        // Tenemos que recivir mensajes de todos los nodos para poder ejecutar la sección crítica
        for (int i = 1; i <= n_procesos; i++){
            // Intentaresmo recivir respuesta de todos los nodos y cuando esto pase iremos a nuestra sección crítica
            if (i != mi_id)
            {
                msgrcv(msg_ack, &msg, sizeof(mensaje), mi_id, 0); //Recivimos el mensajes de tipo ack
                printf("Recivimos el mensaje de confirmación del nodo %li\n",msg.mtype);
            }
        } 
        ///SECCIÓN CRÍTICA;
        printf("Haciendo la sección crítica\n");



        quiero = 0;
        for (int i = 0; i < num_pend; i++){
            // Enviamos los mensajes que nos quedasen pendientes de enviar
            msg.mtype = id_nodos[i];
            msg.id_origen = mi_id;
            msgsnd(msg_ack, &msg, sizeof(mensaje), 0); //Enviamos el mensaje al nodo origen
        }
        num_pend = 0;
    }
}


// Creamos la funcion que se encargará de recivir los mensajes
void* recivir(void *args) {
    mensaje msg;
    printf("Ejecutando hilo\n");
    while (1) {
        
        msgrcv(msg_tickets, &msg, sizeof(mensaje), mi_id, 0);
        printf("Recivimos un mensaje del nodo %li con tipo %i\n",msg.id_origen,msg.ticket_origen);

        // Semaforo de exclusión mutua aquí
        sem_wait(&mutex);
        // asignamos el valor maximo a ticket maximo
        if (msg.ticket_origen > max_ticket){ max_ticket = msg.ticket_origen; }

        if (quiero == 0 || msg.ticket_origen < mi_ticket || (msg.ticket_origen == mi_ticket && (msg.mtype < mi_id))){
            // En caso de que no queramos enviar un ticket quiero = 0
            // En caso de que el ticket recivido sea menor que nuestro ticket
            // Si nuestro ticket es igual al recivido pero nuestro id es mayor que el del origen
            msg.mtype = msg.id_origen;
            msg.id_origen = mi_id;
            msgsnd(msg_ack, &msg, sizeof(mensaje), 0); //Enviamos el mensaje al nodo origen
            printf("Enviamos un mensaje al nodo origen %li\n",msg.mtype);
        }
        else {
            id_nodos_pend[num_pend++] = msg.mtype;
        }
        sem_post(&mutex);
        // Termina el semaforo de exclusion mutua
    }
}
