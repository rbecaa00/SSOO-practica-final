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
	int tiporecepcionista= (int)arg; 
	int posicion = 0;    
	int min= 1; 
	int porcentaje= aleatorios(1, 100); //calculamos numeros aleatorios entre 1 y 100
	//Para los log
	char identificador[50]; 
	char mensaje[200]; 


	//Punto 1 y 2 
	pthread_mutex_lock(&colaClientes);
	
	for(int i=0; i<numClientes; i++){
	
		if(tiporecepcionista == clientes[i].tipo && clientes[i].atendido==0){
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
		}

	//Calculamos el tipo de atencion 
	//Puntos 3 y 4

	if(porcentaje <=80){
		sprintf(identificador, "Cliente %d", clientes[posicion].id); 
		sprintf(mensaje, "El paciente %d tiene todo en regla\n",clientes[posicion].id);
		printf("%s:%s", identificador, mensaje); //Imprimimos lo escrito anteriormente  
		writeLogMessage(identificador, mensaje);
		sleep(aleatorios(1,4)); 
		clientes[posicion].atendido==1; //El cliente ya está atendido.
	}else if(porcentaje >80 && porcentaje <=90){ // Un 10% de los pacientes
		sprintf(identificador, "El cliente %d" , clientes[posicion].id); 
		sprintf(mensaje, "El cliente %d está mal identificado", clientes[posicion].id); 
		writeLogMessage(identificador, mensaje); 
		printf("%s: %s ", identificador, mensaje); 
		sleep(aleatorios(2,6)); 
		clientes[posicion].atendido== 1; //El cliente también ha sido atendido, 

	}else if(porcentaje >90){
		sprintf(identificador, "El cliente %d", clientes[posicion].id); 
		sprintf(mensaje, "El cliente %d no presenta el pasapaorte vacunal" , clientes[posicion].id); 
		writeLogMessage(identificador, mensaje); 
		printf("%s : %s", identificador,  mensaje); 
		sleep(aleatorios(6,10)); 
		clientes[posicion].atendido== -1; //Al no tener el pasaporte vacunal debe abandonar el hotel. 

	}
		
	


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
