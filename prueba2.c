#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

pthread_cond_t condiciones[6];

pthread_mutex_t mutex;

int total;
int vueltas;

int numClie;

int estado;

void *hilo (void *arg){

    pthread_mutex_lock(&mutex);
    printf("Hola\n");
    pthread_mutex_unlock(&mutex);
    //pthread_exit(0);
}

int main(){

    estado = 0;

    total = 0;

    vueltas = 0;

    pthread_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    if (pthread_mutex_init(&mutex, NULL) != 0){
		exit(-1);
	}
    numClie = 0;
    for(int i = 0; i < 6; i++){
        if (pthread_cond_init(&condiciones[i], NULL)!=0) {
            exit(-1); 
        }
    }
    pthread_create(&t1, NULL, hilo,NULL);
    pthread_create(&t2, NULL, hilo,NULL);
    pthread_create(&t3, NULL, hilo,NULL);
    pthread_create(&t4, NULL, hilo,NULL);
    pthread_create(&t5, NULL, hilo,NULL);
    pthread_create(&t6, NULL, hilo,NULL);
    pthread_mutex_lock(&mutex);
    estado = 1;
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&condiciones[0]);
    pthread_create(&t7, NULL, hilo,NULL);

    sleep(1);
    
    pthread_create(&t8, NULL, hilo,NULL);
    pthread_create(&t9, NULL, hilo,NULL);
    pthread_create(&t10, NULL, hilo,NULL);
    printf("Fin\n");
    pthread_exit(NULL);
    //pthread_join();
}

