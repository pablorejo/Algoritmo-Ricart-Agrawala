#include "procesos.h"

int main(int argc, char const *argv[])
{
    


    // Memoria compartida
    int procesos_p_a, procesos_a, procesos_r, procesos_c;
    sem_t sem_aux_variables;
    sem_t sem_mutex; // Semaforo de exclusion mutua con todos los procesos menos los de consultas entre si
    int tenemos_SC; // Variable para comprovar si nuestro nodo tiene la seccion critica
    // Fin memória compartida



    while (1)
    {
        // Quiero entrar en la sección críticia
        // Compruebo que no hay procesos prioritários intentando entrar.
        sem_wait(&sem_aux_variables);
        if (procesos_r == 0  && procesos_a && procesos_p_a){ // En caso de que no haya procesos prioritarios intentando entrar en la SC
            procesos_c ++; 
            if (procesos_c == 0)
            {
                /* code */
                wait(&sem_aux_variables); // Intentamos entrar en la SC
                sem_post(&sem_mutex); 
            }else{
                sem_post(&sem_aux_variables);
            }


            // SECCIÓN CRÍTICA
            #ifdef __PRINT_PROCESO
            printf("Haciendo la SC\n");

            printf("Fin de la SC\n");
            #endif 
            // FIN SECCIÓN CRÍTICA


            wait(&sem_aux_variables);
            procesos_c --;
            if (procesos_c == 0){ // Si ya no hay procesos de consulta ejecutandose se sale de la exclusion mutua
                sem_post(&sem_mutex); // Hacemos que otros procesos puedan entrar en la seccion critica
            }
            sem_post(&sem_aux_variables);
        }
    }
    
    return 0;
}
