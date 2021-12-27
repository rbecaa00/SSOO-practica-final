#include <stdio.h>	
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

pthread_mutex_t fichero;
pthread_mutex_t colaClientes;
pthread_mutex_t ascensor;
pthread_mutex_t maquinas;

struct cliente{
	int id;
	int atendido; // Tipo boolean 0 no atendido 1 atendido
	int tipo; // Tipo boolean 0 no vip 1 vip
	int ascensor; 
};

struct cliente *clientes;

int numClientes = 20;
int contClientes = 0;
int numCliAscensor = 0;


char logFileName[] = "Practica_Final.log";

FILE *logFile;

char ids[20];

void nuevoCliente();
void accionesCliente();
void accionesRecepcionista();
void writeLogMessage(char *id, char *msg);
int aleatorios(int min, int max); 

/**
 * Implementado por Rubén Bécares Álvarez
 */
int main(int argc, char* argv[]){
	
	sprintf(ids, "%d", getpid());
	writeLogMessage("Main", "Inicio programa");

	/**
	 * Montado de señales
	 * 
	 */
	struct sigaction sNV;
	sNV.sa_handler = nuevoCliente;
	if(-1 == sigaction(SIGUSR1, &sNV, NULL)){
		//sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Fallo montando la señal de no vips");
		perror("Fallo montando la señal de no vips");
		return 1;
	}else{
		//sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Se ha montando la señal de no vips");
	}

	struct sigaction sV;
	sV.sa_handler = nuevoCliente;
	if(-1 == sigaction(SIGUSR2, &sV, NULL)){
		//sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Fallo montando la señal de vips");
		perror("Fallo montando la señal de vips");
		return 1;
	}else{
		//sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Se ha montando la señal de vips");
	}

	struct sigaction sC;
	sC.sa_handler = nuevoCliente;
	if(-1 == sigaction(SIGINT, &sC, NULL)){
		//sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Fallo montando la señal de terminar");
		perror("Fallo montando la señal de terminar");
		return 1;
	}else{
		//sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Se ha montando la señal de terminar");
	}

	/**
	 * Inicializacion de recursos
	 * 
	 */

	clientes = (struct cliente *)malloc(sizeof(struct cliente)*numClientes);
	if (pthread_mutex_init(&fichero, NULL)!=0){
		exit(-1); 
	}
	if (pthread_mutex_init(&colaClientes, NULL)!=0){
		exit(-1); 
	}
	if (pthread_mutex_init(&ascensor, NULL)!=0){
		exit(-1); 
	}
	if (pthread_mutex_init(&maquinas, NULL)!=0){
		exit(-1); 
	}
	
	
	free(clientes);

	//sprintf(ids, "%d", getpid());
	writeLogMessage("Main", "Fin programa");
	return 0;
}

void nuevoCliente(){
	
}

//Hilo
void accionesCliente(){
	//Victor escribe aqui
}
//Hilo
void *accionesRecepcionista(void *arg){
	//Buscar al cliente y mirar si es vip o no vip 
	int *recepcionista= (int*)arg; 
	int posicion = 0;    
	int min= 1; 
	int porcentaje= aleatorios(1, 100); //calculamos numeros aleatorios entre 1 y 100
	//Para los log
	char identificador[50]; 
	char mensaje[200]; 


	//Punto 1 y 2 
	pthread_mutex_lock(&colaClientes);
	
	for(int i=0; i<numClientes; i++){
	
		if(recepcionista[0] == clientes[i].tipo && clientes[i].atendido==0){
			if(clientes[i].id!=0 && min < clientes[i].id ){
			min = clientes[i].id; 
			posicion = i; 
			}	

		}
			
	}
		if(min!=0){ //Hay clientes
		clientes[posicion].atendido==1; //Acctualizando 
		}
		pthread_mutex_unlock(&colaClientes); 
		if(min == 0){
			sleep(1); 

		}else{
			
			sprintf(identificador, "Recepcionista_%d", recepcionista[1] ); //Identificador del recepcionista
			sprintf(mensaje, "Comineza la atencion"); 
			writeLogMessage(identificador, mensaje);
		//Se podría poner al inicio del programa, ya que es una de las primeras cosas que hace.

			if(porcentaje <=80){
		sprintf(mensaje, "El cliente %d tiene todo en regla\n",clientes[posicion].id);
		printf("%s:%s", identificador, mensaje); //Imprimimos lo escrito anteriormente 
		sleep(aleatorios(1,4));
		pthread_mutex_lock(&fichero);
		writeLogMessage(identificador, mensaje);
		pthread_mutex_unlock(&fichero); 
		pthread_mutex_lock(&colaClientes);
		clientes[posicion].atendido ==1; //El cliente ya está atendido.
		pthread_mutex_unlock(&colaClientes);

	}else if(porcentaje >80 && porcentaje <=90){ // Un 10% de los pacientes

		sprintf(mensaje, "El cliente %d está mal identificado", clientes[posicion].id); 
		printf("%s: %s ", identificador, mensaje); 
		sleep(aleatorios(2,6)); 
		pthread_mutex_lock(&fichero);
		writeLogMessage(identificador, mensaje); 
		pthread_mutex_unlock(&fichero);
		pthread_mutex_lock(&colaClientes);
		clientes[posicion].atendido == 1; //El cliente también ha sido atendido, 
		pthread_mutex_unlock(&colaClientes);

	}else if(porcentaje >90){
		
		sprintf(mensaje, "El cliente %d no presenta el pasapaorte vacunal" , clientes[posicion].id); 
		pthread_mutex_lock(&fichero);
		writeLogMessage(identificador, mensaje); 
		pthread_mutex_unlock(&fichero);
		printf("%s : %s", identificador,  mensaje); 
		sleep(aleatorios(6,10)); 
		pthread_mutex_lock(&colaClientes);
		clientes[posicion].atendido==0; //Al no tener el pasaporte vacunal debe abandonar el hotel. 
		pthread_mutex_unlock(&colaClientes);
	}else{
		sprintf(mensaje, "Ha finalizado la atención");
		writeLogMessage(identificador, mensaje); 
	}
		

		}

	//Calculamos el tipo de atencion 
	//Puntos 3 y 4



}

int aleatorios(int min, int max){ //Función para calcular numeros aleatorios 
return rand()% (max-min+1) +min; 
}

void writeLogMessage(char *id, char *msg) {
	// Calculamos la hora actual
	 time_t now = time(0);
	 struct tm *tlocal = localtime(&now);
	 char stnow[25];
	 strftime(stnow, 25, "%d/%m/%y %H:%M:%S", tlocal);
	// Escribimos en el log
	 logFile = fopen(logFileName, "a");
	 fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);
	 fclose(logFile);
 }
