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
int acabar;

char logFileName[19];

FILE *logFile;

char ids[20];

void nuevoCliente(int signal);
void *accionesCliente(void *arg);
void *accionesRecepcionista(void *arg);
void writeLogMessage(char *id, char *msg);
int aleatorios(int min, int max); 
void fin();
int size(*lista);
void aumentar();

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
	sC.sa_handler = fin;
	if (-1 == sigaction(SIGINT, &sC, NULL)){
		// sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Fallo montando la señal de terminar");
		perror("Fallo montando la señal de terminar");
		return 1;
	}else{
		// sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Se ha montando la señal de terminar");
	}
	
	struct sigaction sA;
	sA.sa_handler = aumentar;
	if (-1 == sigaction(SIGILL, &sA, NULL)){
		// sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Fallo montando la señal de aumentar");
		perror("Fallo montando la señal de aumentar");
		return 1;
	}else{
		// sprintf(ids, "%d", getpid());
		writeLogMessage("Main", "Se ha montando la señal de aumentar");
	}

	/**
	 * Inicializacion de recursos
	 *
	 */
	if(argc == 2){
		numClientes = atoi(argv[1]);
	}else if(argc == 3){
		numClientes = atoi(argv[1]);
		numMaquinas = atoi(argv[2]);
	}else{
		numClientes = 20;
		numMaquinas = 5;
	}
	
	contClientes = 0;
	numCliAscensor = 0;
	acabar = 0;

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
	int r3[] = {1, 3};
	pthread_create(&recepcionista_1, NULL, accionesRecepcionista, &r1);
	pthread_create(&recepcionista_2, NULL, accionesRecepcionista, &r2);
	pthread_create(&recepcionista_3, NULL, accionesRecepcionista, &r3);

	while (!acabar){
		pause();
	}

	pthread_join(recepcionista_1, NULL);
	pthread_join(recepcionista_2, NULL);
	pthread_join(recepcionista_3, NULL);

	while(size(clientes) != 0){
		sleep(0.1);
	}

	free(clientes);

	// sprintf(ids, "%d", getpid());
	writeLogMessage("Main", "Fin programa");

	return 0;
}

void nuevoCliente(int signal){
	if(acabar==0){
		int posicionCliente=-1;
		//Se bloquea el mutex
		pthread_mutex_lock(&colaClientes);
		//Bucle para asignar la posición en el id
		for(int i=0; i<numClientes;i++){
			if(clientes[i].id==0){
				posicionCliente=i;
			}
		}

		//Caso en el que no caben más clientes
		if(posicionCliente=-1){

			printf("No se admiten más clientes");
		}
		//Caso en el que caben más clientes y se añade un nuevo cliente
		else{
			printf("Hay un nuevo CLIENTE en el hotel");
			contClientes++;
			clientes[posicionCliente].id=contClientes;
			clientes[posicionCliente].atendido=0;
			clientes[posicionCliente].ascensor=0;

			//Diferencia entre clientes VIPS y normales
			switch(signal){
				case SIGUSR1:
					clientes[posicionCliente].tipo=0;
					break;
				case SIGUSR2:
					clientes[posicionCliente].tipo=1;
					break;

			}

			//Se crea un hilo nuevo donde irá el cliente nuevo
			pthread_t aux;
			pthread_create(&aux,NULL,accionesCliente,NULL);
		}
		//Se desbloquea el mutex
		pthread_mutex_unlock(&colaClientes);
	}
}

// Hilo
void *accionesCliente(void *arg){
	char tipo[20];
	char hora[20];
	int id=(int*)cliente;
	int posicionCliente;

	for(int i=0; i<numClientes; i++){
		if(cliente[i].id=id){
			posicionCliente=i;
		}
	}

	sprintf(tipo,"Cliente %d:",id);
	sprintf(hora,"acabo de entrar en el hotel\n");
	pthread_mutex_lock(&fichero);
	writeLogMessage(tipo,hora);
	printf("%s: %s",tipo,hora);
	pthread_mutex_unlock(&fichero);

	while(cliente[posicionCliente].atendido==0){
		int num;
		num = calculaAleatorios(1, 100);
		if(num<=20){
			sprintf(tipo,"Cliente %d:",id);
			sprintf(hora,"Me he cansado de esperar y me voy\n");
			pthread_mutex_lock(&fichero);
			writeLogMessage(tipo, hora);
			printf("%s: %s", tipo, hora);
			pthread_mutex_unlock(&fichero);

		}else if(num>30 && num<=35){
			sprintf(tipo,"Cliente %d:",id);
			sprintf(hora,"He ido al baño y he perdido el truno\n");
			pthread_mutex_lock(&fichero);
			writeLogMessage(tipo,hora);
			printf("%s: %s", tipo, hora);
			pthread_mutex_unlock(&fichero);
			
		}
		pthread_mutex_lock(&colaClientes);
		terminarHiloPaciente(posicionCliente);
		pthread_mutex_unlock(&colaClientes);
		pthread_exit(&cliente[posicionCliente]);
		sleep(3);	
	}

	while(cliente[posicionCliente].ascensor == 0 ){
		sleep(1);
	}
	
}
//Hilo
void *accionesRecepcionista(void *arg){
	//Buscar al cliente y mirar si es vip o no vip 
	int *recepcionista= (int*)arg; 
	int posicion = 0;    
	int min= 1; 
	int porcentaje= aleatorios(1, 100); //calculamos numeros aleatorios entre 1 y 100
	int contador = 0; 
	//Para los log
	char identificador[50]; 
	char mensaje[200]; 


	//Punto 1 y 2 

	while(1){
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
			 //PONER VARIABLE CONDICION PARA CUANDO SE TERMINE EL PROGRAMA 
			sprintf(identificador, "Recepcionista_%d", recepcionista[1] ); //Identificador del recepcionista
			sprintf(mensaje, "Comineza la atencion"); 
			writeLogMessage(identificador, mensaje);
		//Se podría poner al inicio del programa, ya que es una de las primeras cosas que hace.

			if(porcentaje <=80){
		sprintf(mensaje, "El cliente %d tiene todo en regla",clientes[posicion].id);
		printf("%s:%s \n", identificador, mensaje); //Imprimimos lo escrito anteriormente 
		sleep(aleatorios(1,4));
		pthread_mutex_lock(&fichero);
		writeLogMessage(identificador, mensaje);
		pthread_mutex_unlock(&fichero); 
		pthread_mutex_lock(&colaClientes);
		clientes[posicion].atendido ==1; //El cliente ya está atendido.
		pthread_mutex_unlock(&colaClientes);

	}else if(porcentaje >80 && porcentaje <=90){ // Un 10% de los pacientes

		sprintf(mensaje, "El cliente %d está mal identificado", clientes[posicion].id); 
		printf("%s: %s \n", identificador, mensaje); 
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
		printf("%s : %s \n", identificador,  mensaje); 
		sleep(aleatorios(6,10)); 
		pthread_mutex_lock(&colaClientes);
		clientes[posicion].atendido==0; //Al no tener el pasaporte vacunal debe abandonar el hotel. 
		pthread_mutex_unlock(&colaClientes);
	} 
		sprintf(mensaje, "Ha finalizado la atención");
		writeLogMessage(identificador, mensaje); 

		if(recepcionista[0]==0){
		contador+=1; 
			if(contador == 5){
				contador=0; 
				sleep(5); 

			}
		}	

		}
	
		

	}

	//Calculamos el tipo de atencion 
	//Puntos 3 y 4



}

int aleatorios(int min, int max){ //Función para calcular numeros aleatorios 
return rand()% (max-min+1) +min; 
}

void fin(){
	acabar = 1;
}

int size(*lista){
	int res = 0;
	pthread_mutex_lock(&colaClientes);
	for(int i = 0; i < numClientes; i++){
		if(clientes[i].id != 0){
			res++;
		}
	}
	pthread_mutex_unlock(&colaClientes);
	return res;
}

void aumentar(){
	int num = 0;
	int aumento = 0;
	int tries = 0
	do{
	printf("¿Que desea aumentar?\n");
	printf("1)Numero de clientes\n");
	printf("2)Numero de maquinas\n");
	tries++;
	scanf( "%d", &num);
	}while(num!= 1 && num!=2 && tries < 2);

	if(num == 1 || num == 2){
		printf("Indique en cuanto: ");
		scanf("%d", &aumento);

		switch (num){
		case 1:
			/* Redimensionamos el puntero y lo asignamos a un puntero temporal */
			pthread_mutex_lock(&colaClientes);
			struct clientes *tmp_ptr = (struct clientes *)realloc(clientes, sizeof(struct clientes)*(num+numClientes));

			if (tmp_ptr == NULL) {
				printf("Hubo un error en el aumento de capacidad");
				perror("Hubo un error en el aumento de capacidad\n");
			}
			else {
				/* Reasignación exitosa. Asignar memoria a clientes */
				clientes = tmp_ptr;
				numClientes += num;
			}
			pthread_mutex_unlock(&colaClientes);
			pthread_mutex_lock(&fichero);
			writeLogMessage("Main","Aumento de clientes a %d", numClientes);
			pthread_mutex_unlock(&fichero);

			break;
		case 2:
			/* Redimensionamos el puntero y lo asignamos a un puntero temporal */
			pthread_mutex_lock(&maquinasCheckin);
			int *tmp_ptr = (int*)realloc(clientes, sizeof(int)*(num+numMaquinas));

			if (tmp_ptr == NULL) {
				printf("Hubo un error en el aumento de capacidad");
				perror("Hubo un error en el aumento de capacidad\n");
			}
			else {
				/* Reasignación exitosa. Asignar memoria a clientes */
				clientes = tmp_ptr;
				numMaquinas += num;
			}
			pthread_mutex_unlock(&maquinasCheckin);
			pthread_mutex_lock(&fichero);
			writeLogMessage("Main","Aumento de maquinas a %d", numMaquinas);
			pthread_mutex_unlock(&fichero);
			break;
		}

	}else{
		printf("Demasiados intentos incorrectos\n");
	}

	

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
