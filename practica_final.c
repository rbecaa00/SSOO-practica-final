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
	int atentido; // Tipo boolean 0 no atendido 1 atendido
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
void accionesRecepcionista(){
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
