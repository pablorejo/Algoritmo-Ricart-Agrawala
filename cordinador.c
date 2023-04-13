#include "procesos.h"


int msg_recibir_id, id_nodos[N] = {0}, id_ultimonodo = 0;


mensaje msg_recibir;

int main(int argc, char const *argv[])
{
    

    key_t key = ftok("recibir.c",1);
    
    msg_recibir_id = msgget(key,0660 | IPC_CREAT); // Creamos el buzón
    

    msgrcv(msg_recibir_id, &msg_recibir, sizeof(mensaje), ID_NODO_CONTROLADOR, 0); // Recivimos los mensajes que nos llegan de los nodos

    #ifdef __PRINT
    printf("Recivimos un mensaje del nodo %i con tipo %li y el ticket es %i\n",msg_recibir.id_origen,msg_recibir.mtype,msg_recibir.ticket_origen);
    #endif 
    

    
    while (1)
    {
    
        if (msg_recibir.opcion == DAR_ALTA_NODO)
        {
            int id_nodo_origen = msg_recibir.id_origen;
            // Damos de alta al nodo
            id_ultimonodo ++;
            id_nodos[id_ultimonodo] = msg_recibir.id_origen;


            // Enviamos mensajes a todos los nodos
            for (int i = 0; i < id_ultimonodo-1; i++)
            {
                if (id_nodos[i] != 0) // Comprobamos que el nodo existe
                {
                    msg_recibir.mtype = id_nodos[i]; // Dirección destino la del nodo
                    msg_recibir.id_origen = ID_NODO_CONTROLADOR; // Dirección origen la del nodo controlador
                    msg_recibir.ticket_origen = id_nodo_origen; 
                    msg_recibir.opcion = DAR_ALTA_NODO; // Tipo de mensaje dar de alta
                    msgsnd(msg_recibir_id, &msg_recibir, sizeof(mensaje), 0); //Enviamos el ticket al nodo 
                }
            }

            #ifdef __PRINT
            printf("Se ha dado de alta al nodo %i", id_nodos[id_ultimonodo] );
            #endif

        } // Fin dar de alta




        else if( msg_recibir.opcion == DAR_BAJA_NODO){

            int id_nodo_baja = msg_recibir.id_origen;
            // Enviamos mensajes a todos los nodos
            for (int i = 0; i < id_ultimonodo; i++)
            {
                if (id_nodos[i] != 0) // Comprobamos que el nodo existe
                {
                    if (id_nodos[i] == id_nodo_baja)
                    {
                        id_nodos[i] = 0;
                    }else{

                        msg_recibir.mtype = id_nodos[i]; // Dirección destino la del nodo
                        msg_recibir.id_origen = ID_NODO_CONTROLADOR; // Dirección origen la del nodo controlador
                        msg_recibir.ticket_origen = id_nodo_baja;
                        msg_recibir.opcion = DAR_BAJA_NODO; // Tipo de mensaje dar de baja
                        msgsnd(msg_recibir_id, &msg_recibir, sizeof(mensaje), 0); //Enviamos el ticket al nodo 
                    }
                }
            }
            #ifdef __PRINT
            printf("Se ha dado de baja al nodo %i", id_nodo_baja);
            #endif
        } // Fin dar de baja
    }
    return 0;
}
