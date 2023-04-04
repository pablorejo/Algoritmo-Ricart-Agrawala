#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/types.h> // Se definen algunos tipos de datos
#include <sys/ipc.h> // Para algunas banderas
#include <sys/msg.h> // Para usar la funcion

#define N 3 
#define MAX_MSG_SIZE 1000

int mi_ticket = 0, mi_id, id_nodos_pend[N-1] = {0}, num_pend = 0, quiero = 0, max_ticket = 0;

pthread_t mi_id_thread;

int msgid; //id del buzón

void* recivir(void *);

typedef struct 
{
    int id_nodo_origen;
    int ticket_origen;
}mensaje;

mensaje msg;

int main(int argc, char const *argv[])
{
    if (argc < 2){
        mi_id = argv[1]; // Guardamos el id de nuestro que nos otorgara el usuario
    }else
    {
        printf("Introduce el id\n");
        exit(-1);
    }
    int msgid = msgget(1,0664 | IPC_CREAT); // Creamos el buzón
    // pthread_create(, NULL, recivir, (void *) );
    int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);

    while (1)
    {
        for (int i = 0; i < N-1; i++) send(REQUEST, id_nodos[i], {mi_id, mi_ticket});
        for (int i = 0; i < N-1; i++) receive(REPLY, &id_aux);
        SECCIÓN CRÍTICA;
        quiero = 0;
        for (i = 0; i < num_pend; i++)
        // send(REPLY, id_nodos_pend[i], mi_id);
        num_pend = 0;
    }

}


// Creamos la funcion que se encargará de recivir los mensajes
void* recivir(void *args) {

    while (1) {
        
        msgrcv(msgid, &msg, MAX_MSG_SIZE, mi_id, 0);
        if (msg.ticket_origen < mi_ticket){
            msgsnd(msgid, &msg, 2*sizeof(int), msg.id_nodo_origen); //Enviamos el mensaje al nodo origen
        } 
        else{
            id_nodos_pend[num_pend++] = msg.id_nodo_origen ;
        }

        max_ticket = MAX(max_ticket, ticket_origen);
        if (NOT quiero OR ticket_origen < mi_ticket || (ticket_origen == mi_ticket && (id_nodo_origen < mi_id))
        send(REPLY, id_nodo_origen, mi_id);
        else id_nodos_pend[num_pend++] = id_nodo_origen;
    }
}
