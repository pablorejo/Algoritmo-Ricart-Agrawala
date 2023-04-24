#include "../procesos.h"

int pid;


memoria_compartida *mem;
sem_t sem_sync_consultas;
sem_t sem_mutex_consultas;
int procesos_c_espera, procesos_c;

int main(int argc, char const *argv[])
{
    int keyNodo;
    if (argc < 2){
        #ifndef DEBUG
            printf("Error introduce el numero de id de su nodo");
            exit(-1);   
        #endif // !DEBUG

        #ifdef DEBUG
            keyNodo = 1;
        #endif // DEBUG

    }else {
        keyNodo = atoi(argv[1]); // tiene que ser igual a la id del nodo
        sem_init(&sem_sync_consultas,0,0); //Semáforo de paso entre consultas
        sem_init(&sem_mutex_consultas,0,1); //Semáforo de exclusión mutua entre consultas 
    }

    pid = getpid();

    #ifdef __PRINT_PROCESO
    printf("Soy el proceso con pid %i\n",pid);
    #endif 
    


    key_t key = ftok(".",1);

    int permisos = 0666; // permisos de lectura/escritura para todos los usuarios
    int msg_memoria_id = shmget(key+keyNodo, sizeof(memoria_compartida), permisos | IPC_CREAT);
    mem = shmat(msg_memoria_id, NULL, 0);

    #ifdef __PRINT_PROCESO
    printf("Key: %i y id de la memoria compartida es %i\n",key,msg_memoria_id);
    #endif 

    while(1){
        if(procesos_c == 0){
            //SOY EL PRIMERO -> TENGO QUE AVISAR AL RESTO DE PROCESOS DE CONSULTAS
            sem_wait(&(mem->procesos_c_pend)); /// Esto no tiene sentido ya que no es un semaforo
            sem_wait(&(mem->sem_aux_variables));
            procesos_c ++;
            mem->procesos_c_pend ++; // Indicamos que el de consultas desea entrar
            
            if (mem->tenemos_SC == 0){
                sem_post(&(mem->sem_sync_intentar)); // Innetamos entrar en la seccion 
            }
            sem_post(&(mem->sem_aux_variables));

            //Nos dejan entrar en la SC
            sem_wait(&(mem->sem_consultas));
            
            // AVISO AL RESTO DE QUE ME DEJARON PASAR:
            sem_post(&(sem_sync_consultas));

            // SECCIÓN CRÍTICA
            sem_wait(&(mem->sem_aux_variables));
            mem->procesos_c_pend --;
            sem_post(&(mem->sem_aux_variables));
            sem_post(&sem_sync_consultas);

            #ifdef __PRINT_SC
            printf("Haciendo la SC\n");
            sleep(SLEEP);
            printf("Fin de la SC\n");
            #endif 
            // FIN SECCIÓN CRÍTICA: lo marca el último

            sem_wait(&sem_mutex_consultas);
            mem->procesos_c_pend --;
            sem_post(&sem_mutex_consultas);

        }else{
            //NO SOY EL PRIMERO -> TENGO QUE ESPERAR A QUE ME DEJE ENTRAR EL 1ER CONSULTAS
            sem_wait(&sem_mutex_consultas);
            procesos_c ++;
            procesos_c_espera++;    //Para saber cuantos procesos de espera están esperando
            sem_post(&sem_mutex_consultas);

            sem_wait(&sem_sync_consultas);  //Espero a que la primera consulta me deje pasar

            sem_wait(&sem_mutex_consultas);
            procesos_c_espera --;
            mem->procesos_c_pend ++;
            sem_post(&sem_mutex_consultas);

            // SECCIÓN CRÍTICA
            #ifdef __PRINT_SC
            printf("Haciendo la SC\n");
            sleep(SLEEP);
            printf("Fin de la SC\n");
            #endif 
            // FIN SECCIÓN CRÍTICA

            sem_wait(&sem_mutex_consultas);
            mem->procesos_c_pend --;
            sem_post(&sem_mutex_consultas);
        }
        
        sem_wait(&sem_mutex_consultas);
        procesos_c --;
        sem_post(&sem_mutex_consultas);

        if(mem->procesos_c_pend == 0){
            //YA NO HAY MÁS PROCESOS DE CONSULTAS EN COLA, DEJAMOS PASAR AL RESTO
            sem_post(&(mem->sem_sync_end)); // Hacemos que otros procesos puedan entrar en la seccion critica
        }
    }
    return 0;
}
