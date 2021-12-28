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
int *maquinasCheckin;

int numClientes;
int contClientes;
int numCliAscensor;
int numMaquinas;

char logFileName[19];

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
int main(int argc, char *argv[]){
	/**
	 * Inicialización nombre fichero
	 *
	 */
	sprintf(logFileName, "Practica_Final.log");

	// sprintf(ids, "%d", getpid());
	writeLogMessage("Main", "Inicio programa");

	/**
	 * Montado de señales
	 *
	 */
	struct sigaction sNV;
	sNV.sa_handler = nuevoCliente;
	if (-1 == sigaction(SIGUSR1, &sNV, NULL)){
		// sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Fallo montando la señal de no vips");
		perror("Fallo montando la señal de no vips");
		return 1;
	}else{
		// sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Se ha montando la señal de no vips");
	}

	struct sigaction sV;
	sV.sa_handler = nuevoCliente;
	if (-1 == sigaction(SIGUSR2, &sV, NULL)){
		// sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Fallo montando la señal de vips");
		perror("Fallo montando la señal de vips");
		return 1;
	}else{
		// sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Se ha montando la señal de vips");
	}

	struct sigaction sC;
	sC.sa_handler = nuevoCliente;
	if (-1 == sigaction(SIGINT, &sC, NULL)){
		// sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Fallo montando la señal de terminar");
		perror("Fallo montando la señal de terminar");
		return 1;
	}else{
		// sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Se ha montando la señal de terminar");
	}

	/**
	 * Inicializacion de recursos
	 *
	 */

	numClientes = 20;
	contClientes = 0;
	numCliAscensor = 0;
	numMaquinas = 5;

	clientes = (struct cliente *)malloc(sizeof(struct cliente) * numClientes);
	maquinasCheckin = (int *)malloc(sizeof(int) * numMaquinas);

	for (int i = 0; i < numClientes; i++){
		clientes[i].id = 0;
		clientes[i].tipo = 0;
		clientes[i].atendido = 0;
		clientes[i].ascensor = 0;
	}

	if (pthread_mutex_init(&fichero, NULL) != 0){
		exit(-1);
	}
	if (pthread_mutex_init(&colaClientes, NULL) != 0){
		exit(-1);
	}
	if (pthread_mutex_init(&ascensor, NULL) != 0){
		exit(-1);
	}
	if (pthread_mutex_init(&maquinas, NULL) != 0){
		exit(-1);
	}

	pthread_t recepcionista_1, recepcionista_2, recepcionista_3;
	int r1[] = {0, 1};
	int r2[] = {0, 2};
	int r2[] = {1, 3};
	pthread_create(&recepcionista_1, NULL, accionesRecepcionista, &r1);
	pthread_create(&recepcionista_2, NULL, accionesRecepcionista, &r2);
	pthread_create(&recepcionista_3, NULL, accionesRecepcionista, &r3);

	while (1){
		pause();
	}

	free(clientes);

	// sprintf(ids, "%d", getpid());
	writeLogMessage("Main", "Fin programa");
	return 0;
}

void nuevoCliente(int signal){
	
	int posicionCliente=-1;
	//Se bloquea el mutex
	pthread_mutex_lock(&colaClientes);
	//Bucle para asignar la posición en el id
	for(int i=0; i<numClientes;i++){
		if(cliente[i].id==0){
			posicionCliente=i;
		}
	}

	//Caso en el que no caben más clientes
	if(posicionCliente=-1){
		printf("No se admiten más clientes")
	}
	//Caso en el que caben más clientes y se añade un nuevo cliente
	else{
		printf("Hay un nuevo CLIENTE en el hotel");
		contClientes++;
		cliente[posicionCliente].id=contClientes;
		cliente[posicionCliente].atendido=0;
		cliente[posicionCliente].ascensor=0;

		//Diferencia entre clientes VIPS y normales
		switch(signal){
			case SIGUSR1:
				cliente[posicionCliente].tipo=1;
				break;
			case SIGUSR2:
				cliente[posicionCliente].tipo=2;
				break;

		}

		//Se crea un hilo nuevo donde irá el cliente nuevo
		pthread_t aux;
		pthread_create(&aux,NULL,accionesCliente,NULL);
	}
	//Se desbloquea el mutex
	pthread_mutex_unlock(&colaClientes);
}

// Hilo
void accionesCliente(){
	// Victor escribe aqui
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

void writeLogMessage(char *id, char *msg){
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
