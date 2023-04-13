#include "procesos.h"


int msg_tickets_id;

mensaje msg_recibir;

int main(int argc, char const *argv[])
{
    

    key_t key = ftok("recibir.c",1);
    
    msg_tickets_id = msgget(key,0660 | IPC_CREAT); // Creamos el buzón
    

    msgrcv(msg_tickets_id, &msg_recibir, sizeof(mensaje), ID_NODO_CONTROLADOR, 0); // Recivimos los mensajes que nos llegan de los nodos

    #ifdef __PRINT
    printf("Recivimos un mensaje del nodo %i con tipo %li y el ticket es %i\n",msg_recibir.id_origen,msg_recibir.mtype,msg_recibir.ticket_origen);
    #endif 
    




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
        // printf("Enviamos un mensaje al nodo origen %li\n",msg_recibir.mtype);



    }else if (msg_recibir.ticket_origen == ACK) // Comprovamos que el ticket no es un ack
    {
        ack_recividos--; 

        #ifdef __PRINT
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

    return 0;
}
