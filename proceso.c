#include "procesos.h" // Incluimos la cabecera de los procesos

int pid;
semaforo msg_semaforo;

int main(int argc, char const *argv[])
{
    int keyNodo = atoi(argv[1]); // tiene que ser igual a la id del nodo
    pid = getpid();

    #ifdef __PRINT_PROCESO
    printf("Soy el proceso con pid %i\n",pid);
    #endif 

    int msg_semaforo_id;
    key_t key = ftok("recibir.c",1);
    msg_semaforo_id = msgget(key+keyNodo,0660 | IPC_CREAT); // Creamos el buzón


    #ifdef __PRINT_PROCESO
    printf("Key: %i y id del buzon %i\n",key+keyNodo,msg_semaforo_id);
    #endif 


    while (1)
    {
        // while (getchar() != '\n') {} // Esperamos a que se introduzca un enter
        sleep(SLEEP);
        // printf("El proceso %i está intentando entrar en la SC\n",pid);


        
        msgrcv(msg_semaforo_id, &msg_semaforo, sizeof(semaforo), SEM_MUTEX, 0); //Para entrar en exclusion mutua con otros procesos del mismo nodo
        // printf("Intentando entran en la sección crítica\n");

        msg_semaforo.mtype = SEM_SYNC_INTENTAR;
        msgsnd(msg_semaforo_id, &msg_semaforo, sizeof(semaforo), 0); // Intentamos entrar en la seccion critica avisamos al recividor
       

        msgrcv(msg_semaforo_id, &msg_semaforo, sizeof(semaforo), SEM_SYNC_INIT, 0); // Recivimos sincronizacion para entrar en la seccion critcia
        
        #ifdef __PRINT_SC
        
        sleep(SLEEP);
        // SECION CRÍTICA
        printf("El proceso %i está en la sección crítica\n",pid);
        // while (getchar() != '\n') {} // Esperamos a que se introduzca un enter
        sleep(SLEEP);
        // printf("Realizando la seccion críticia\n");
        sleep(SLEEP);
        // while (getchar() != '\n') {} // Esperamos a que se introduzca un enter
        printf("El proceso %i abandonó la sección crítica\n",pid);
        // TERMINA LA SECCIÓN CRÍTICA

        #endif // DEBUG


        msg_semaforo.mtype = SEM_SYNC_END;
        msgsnd(msg_semaforo_id, &msg_semaforo, sizeof(semaforo), 0);// Enviamos sincronizacion de que terminamos la seccion crítica
        
        msg_semaforo.mtype = SEM_MUTEX;
        msgsnd(msg_semaforo_id, &msg_semaforo, sizeof(semaforo), 0); // Permitimos a otros entrar en la seccion crítica
        /* code */
    }
    
    return 0;
}
