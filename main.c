#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/types.h> // Se definen algunos tipos de datos
#include <sys/ipc.h> // Para algunas banderas
#include <sys/msg.h> // Para usar la funcion

#include <semaphore.h>
#define N 1000

int mi_ticket = 0, mi_id, id_nodos_pend[N-1] = {0}, num_pend = 0, quiero = 0, max_ticket = 0, n_procesos = N-1;
int msgid; //id del buzón

int id_nodos[N-1] = {0};//Id de todos los nodos


typedef struct 
{
    int id_nodo_origen;
    int ticket_origen;
}mensaje;
mensaje msg;

sem_t mutex;
pthread_t mi_id_thread;


void* recivir(void *);

int main(int argc, char const *argv[])
{
    if (argc < 3){
        printf("Introduce el id y cuantos procesos hay\n");
        exit(-1);
    }else{
        mi_id = atoi(argv[1]); // Guardamos el id que nos otorgara el usuario    
        n_procesos = atoi(argv[2]); // Numero de procesos totales

        // Guardando ids de los procesos
        for (int i = 0; i < n_procesos; i++){
            id_nodos[i] = i;
        }
    }
    int msgid = msgget(1,0664 | IPC_CREAT); // Creamos el buzón


    msg.id_nodo_origen = mi_id;
    msg.ticket_origen = 0;

    // iniciamos el semáforo
    sem_init(&mutex,0,1);


    int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
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
                msgsnd(msgid, &msg, 2*sizeof(int), id_nodos[i]); //Enviamos el mensaje al nodo origen
            }
        }
        printf("Reciviendo mensajes para a todos los nodos\n");

        // Tenemos que recivir mensajes de todos los nodos para poder ejecutar la sección crítica
        for (int i = 0; i < n_procesos; i++){
            // Intentaresmo recivir respuesta de todos los nodos y cuando esto pase iremos a nuestra sección crítica
            msgrcv(msgid, &msg, 2*sizeof(int), mi_id, 0); //Recivimos el mensaje
        } 
        ///SECCIÓN CRÍTICA;
        printf("Haciendo la sección crítica\n");



        quiero = 0;
        for (int i = 0; i < num_pend; i++){
            // Enviamos los mensajes que nos quedasen pendientes de enviar
            msgsnd(msgid, &msg, 2*sizeof(int), i); //Enviamos el mensaje al nodo origen
        }
        num_pend = 0;
    }
}


// Creamos la funcion que se encargará de recivir los mensajes
void* recivir(void *args) {
    while (1) {
        // recivimos una petición
        msgrcv(msgid, &msg, 2*sizeof(int), mi_id, 0);


        // Semaforo de exclusión mutua aquí
        sem_wait(&mutex);
        // asignamos el valor maximo a ticket maximo
        if (msg.ticket_origen > max_ticket){ max_ticket = msg.ticket_origen; }

        if (quiero == 0 || msg.ticket_origen < mi_ticket || (msg.ticket_origen == mi_ticket && (msg.id_nodo_origen < mi_id))){
            // En caso de que no queramos enviar un ticket quiero = 0
            // En caso de que el ticket recivido sea menor que nuestro ticket
            // Si nuestro ticket es igual al recivido pero nuestro id es mayor que el del origen
            msgsnd(msgid, &msg, 2*sizeof(int), msg.id_nodo_origen); //Enviamos el mensaje al nodo origen
        }
        else {
            id_nodos_pend[num_pend++] = msg.id_nodo_origen;
        }
        sem_post(&mutex);
        // Termina el semaforo de exclusion mutua
    }
}
