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
int n[8] = {1,2,3,4,5,6,7,8};

int numClie;

int estado;

void *hilo (void *arg){
    int d = *(int*)arg;

    printf("Antes zona critica %d\n",d);
    pthread_mutex_lock(&mutex);
    while(estado == 0 /*|| numClie == 5*/){
        printf("Esperando%d\n",d);
        pthread_cond_wait(&condiciones[0], &mutex);
    }
    printf("Zona critica%d\n",d);
    printf("%d\n",++total);
    printf("Estado:%d\n",estado);
    printf("Vueltas:%d\n",vueltas);
    int aux = ++numClie;
    if(numClie>=5){
        estado = 0;
        printf("Trabajando\n\n");
        pthread_mutex_unlock(&mutex);
        sleep(1);
        pthread_mutex_lock(&mutex);
        vueltas++;
    }else{
        pthread_cond_wait(&condiciones[aux], &mutex);
    }
    numClie--;
    pthread_cond_broadcast(&condiciones[numClie]);
    if(numClie == 0){
        estado = 1;
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(0);


}

int main(){

    estado = 0;

    total = 0;

    vueltas = 0;


    pthread_t t1, t2, t3, t4 ,t5 ,t6, t7, t8;
    if (pthread_mutex_init(&mutex, NULL) != 0){
		exit(-1);
	}

    numClie = 0;

    for(int i = 0; i < 6; i++){
        if (pthread_cond_init(&condiciones[i], NULL)!=0) {
            exit(-1); 
        }
    }
    pthread_create(&t1, NULL, hilo,&n[0]);
    pthread_create(&t2, NULL, hilo,&n[1]);
    pthread_create(&t3, NULL, hilo,&n[2]);
    pthread_create(&t4, NULL, hilo,&n[3]);
    pthread_create(&t5, NULL, hilo,&n[4]);
    pthread_create(&t6, NULL, hilo,&n[5]);
    pthread_mutex_lock(&mutex);
    estado = 1;
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&condiciones[0]);
    pthread_create(&t7, NULL, hilo,&n[6]);
    sleep(1);
    pthread_create(&t8, NULL, hilo,&n[7]);
    pthread_exit(NULL);
}

/*
numAscensor = 6;

...

numAscensor--;

while(numAscenso < 0){
    wait(condicion[0])
}

...

numAscensor++;




...



while(estado == 0){
    wait(condicion[0])
}

mutex

numAscensor++;

if(numAscensor == 6){
    en marchar
    
}else{
    wait(condicion[cliente.ascensor])
    
}

if(numAscensor == 1){
    estado = 1
}
numAscensor--;
signal(condicion[cliente.ascensor-1])
exit

...*/