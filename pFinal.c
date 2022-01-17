#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//Declaraciones globales: mutex
pthread_mutex_t fichero; 
pthread_mutex_t colaClientes;
pthread_mutex_t ascensor;
pthread_mutex_t maquinas;
pthread_mutex_t finalizar;

// Struct de cliente con sus características
//Lista de 20 clientes
struct cliente {
    int id;
    int atendido; // 0=esperando recepcionista; 1=esperando maquina; 2=haciendo checkin; 3=atendido; 4=se va
    int tipo; // Tipo boolean 0 no vip 1 vip
    int ascensor;
};

//Condiciones para el ascensor y los clientes que deben subir
int numClientesAscensor;
pthread_cond_t conAscensor[6];
int estadoAscensor;

//Contador de clientes y maquinas (en forma de puntero/lista)
struct cliente* clientes;
int* maquinasCheckin;

int numClientes;
int contClientes;
int numMaquinas;
int acabar; //Condicion para acabar la ejecución del programa 

//Nombre del fichero log donde se guarda la ejecución del programa
char logFileName[19]; 

FILE* logFile;

char ids[20];


//Definimos las funciones
void nuevoCliente(int signal);
void* accionesCliente(void* arg);
void* accionesRecepcionista(void* arg);
void writeLogMessage(char* id, char* msg);
int aleatorios(int min, int max);
void fin();
int sizeClientes();
void aumentar(); //Aumentar maquinas y clientes en tiempo de ejecución

/**
 * Funcion main y receptor de señales
 */
int main(int argc, char* argv[]){
    /**
     * @brief 
     * 
     */
    sprintf(logFileName, "Practica_Final.log");

    // sprintf(ids, "%d", getpid());
    writeLogMessage("Main", "Inicio programa");

    numClientesAscensor = 0;

    /**
     * Montado de señales
     *
     */
     //Cliente no vip (SIGUSR1)
    struct sigaction sNV;
    sNV.sa_handler = nuevoCliente;
    sNV.sa_flags = 0;
    sigemptyset(&sNV.sa_mask);
    if (-1 == sigaction(SIGUSR1, &sNV, NULL)) {
        // sprintf(ids, "%d", getpid());
        writeLogMessage("Main", "Fallo montando la señal de no vips");
        perror("Fallo montando la señal de no vips");
        return 1;
    } else {
        // sprintf(ids, "%d", getpid());
        writeLogMessage("Main", "Se ha montando la señal de no vips");
    }
    
    //Cliente vip (SIGUSR2)
    struct sigaction sV;
    sV.sa_handler = nuevoCliente;
    sV.sa_flags = 0;
    sigemptyset(&sV.sa_mask);
    if (-1 == sigaction(SIGUSR2, &sV, NULL)) {
       
        writeLogMessage("Main", "Fallo montando la señal de vips");
        perror("Fallo montando la señal de vips");
        return 1;
    } else {
       
        writeLogMessage("Main", "Se ha montando la señal de vips");
    }

    //Señal para terminar (SIGINT)
    struct sigaction sC;
    sC.sa_handler = fin;
    sC.sa_flags = 0;
    sigemptyset(&sC.sa_mask);
    if (-1 == sigaction(SIGINT, &sC, NULL)) {
       
        writeLogMessage("Main", "Fallo montando la señal de terminar");
        perror("Fallo montando la señal de terminar");
        return 1;
    } else {
       
        writeLogMessage("Main", "Se ha montando la señal de terminar");
    }

   //Parte obtativa, aumenar clientes y maquinas en tiempo de ejecución
    struct sigaction sA;
    sA.sa_handler = aumentar;
    sA.sa_flags = 0;
    sigemptyset(&sA.sa_mask);
    if (-1 == sigaction(SIGILL, &sA, NULL)) {
       
        writeLogMessage("Main", "Fallo montando la señal de aumentar");
        perror("Fallo montando la señal de aumentar");
        return 1;
    } else {
      
        writeLogMessage("Main", "Se ha montando la señal de aumentar");
    }

    /**
     * Inicializacion de recursos
     *
     */
    if (argc == 2) {
        numClientes = atoi(argv[1]);
    } else if (argc == 3) {
        numClientes = atoi(argv[1]);
        numMaquinas = atoi(argv[2]);
    } else {
        numClientes = 20;
        numMaquinas = 5;
    }

    estadoAscensor = 1;
    contClientes = 0;
    acabar = 0;

    clientes = (struct cliente*)malloc(sizeof(struct cliente) * numClientes);
    maquinasCheckin = (int*)malloc(sizeof(int) * numMaquinas);

    for (int i = 0; i < numClientes; i++) {
        clientes[i].id = 0;
        clientes[i].tipo = 0;
        clientes[i].atendido = 0;
        clientes[i].ascensor = 0;
    }

    /**
    Maquinas de Checking ponerlas todas libres
    */
    for (int i = 0; i < numMaquinas; i++) {
        maquinasCheckin[i] = 0;
    }

    if (pthread_mutex_init(&fichero, NULL) != 0) {
        exit(-1);
    }
    if (pthread_mutex_init(&colaClientes, NULL) != 0) {
        exit(-1);
    }
    if (pthread_mutex_init(&ascensor, NULL) != 0) {
        exit(-1);
    }
    if (pthread_mutex_init(&maquinas, NULL) != 0) {
        exit(-1);
    }
    if (pthread_mutex_init(&finalizar, NULL) != 0) {
        exit(-1);
    }

    for (int x = 0; x <= 5; x++) {
        if (pthread_cond_init(&conAscensor[x], NULL) != 0) {
            exit(-1);
        }
    }
     /**
     Creacion de los hilos de los recepcionistas

     */
    pthread_t recepcionista_1, recepcionista_2, recepcionista_3;
    int r1[] = { 0, 1 };
    int r2[] = { 0, 2 };
    int r3[] = { 1, 3 };
    pthread_create(&recepcionista_1, NULL, accionesRecepcionista, &r1);
    pthread_create(&recepcionista_2, NULL, accionesRecepcionista, &r2);
    pthread_create(&recepcionista_3, NULL, accionesRecepcionista, &r3);


    /**
    Esperando signals de forma infinita 
    */
    while (!acabar) {
        pause();
    }
     /**
    Sincronizacion entre hilos 
     */
    pthread_join(recepcionista_1, NULL);
    pthread_join(recepcionista_2, NULL);
    pthread_join(recepcionista_3, NULL);

    while (sizeClientes() != 0) {
        sleep(0.1);
    }

    free(clientes);
    free(maquinasCheckin); 

   
    writeLogMessage("Main", "Fin programa");

    return 0;
}


/**

FUNCION NUEVO CLIENTE

*/

void nuevoCliente(int signal) {
    int fin;

    //Protegemos la varible global con un mutex
    pthread_mutex_lock(&finalizar);
    fin = acabar;
    pthread_mutex_unlock(&finalizar);

    //Comprobamos si hay espacion en la cola de clientes
    if (fin == 0) {
        int posicionCliente = -1;
        // Se bloquea el mutex
        pthread_mutex_lock(&colaClientes);
        // Bucle para asignar la posición en el id
        for (int i = 0; i < numClientes && posicionCliente == -1; i++) {
            if (clientes[i].id == 0) {
                posicionCliente = i;
            }
        }

        // Caso en el que no caben más clientes
        if (posicionCliente == -1) {
            
        }
        // Caso en el que caben más clientes y se añade un nuevo cliente
        else {
            contClientes++;
            clientes[posicionCliente].id = contClientes;
            clientes[posicionCliente].atendido = 0;
            clientes[posicionCliente].ascensor = 0;

            // Diferencia entre clientes VIPS y normales
            switch (signal) {
            case SIGUSR1:
                
                clientes[posicionCliente].tipo = 0;
                break;
            case SIGUSR2:
            
                clientes[posicionCliente].tipo = 1;
                break;
            }

            // Se crea un hilo nuevo donde ira el cliente nuevo
            //NOTA: Si no hay espacio se ignora la llamada
            pthread_t aux;
            int r1[] = { posicionCliente };
            pthread_create(&aux, NULL, accionesCliente, &r1);
        }
        // Se desbloquea el mutex
        pthread_mutex_unlock(&colaClientes);
    }
}

// Hilo

/**

ACCIONES CLIENTE

*/
void* accionesCliente(void* arg) {
    char identificador[20];
    char mensaje[75];
    int posicionCliente = ((int*)arg)[0];
    
    int checkin = 0;
    int waiting = 1;

    int posAs;
    int fin;
    int espAs = 0;
    int atendido;

    pthread_mutex_lock(&colaClientes);
    int id = clientes[posicionCliente].id;
    pthread_mutex_unlock(&colaClientes);

    // sprintf sirve para crear una cadena y guardarla en una variable
    sprintf(identificador, "Cliente %d", id);
    sprintf(mensaje, "acabo de entrar en el hotel");
    // Lock del mutex para que el writeMessage vaya uno a uno al log
    pthread_mutex_lock(&fichero);
    writeLogMessage(identificador, mensaje);
    printf("%s: %s\n", identificador, mensaje);
    // Unlock del mutex
    pthread_mutex_unlock(&fichero);

    int num, maquinasCheckinVariable;
    int i = 0;
    // Probabilidad cliente
    num = aleatorios(1, 100);

    if (num <= 10) {
        checkin = 1;
    }


    /**
    Comprueba si el cliente ha sido atendido
    */
    pthread_mutex_lock(&colaClientes);
    atendido = clientes[posicionCliente].atendido;
    pthread_mutex_unlock(&colaClientes);

    while (atendido < 2) {
        num = aleatorios(1, 100);

        if (checkin == 0) {
            if (num <= 20) {
                checkin = 1;
                // sprintf(identificador,"Cliente %d:",id);
                sprintf(mensaje, "Me he cansado de esperar y me voy a las maquinas de Checking");

                pthread_mutex_lock(&colaClientes);
                clientes[posicionCliente].atendido = 1;
                pthread_mutex_unlock(&colaClientes);

                pthread_mutex_lock(&fichero);
                writeLogMessage(identificador, mensaje);
                printf("%s: %s\n", identificador, mensaje);
                pthread_mutex_unlock(&fichero);

            } else if (num > 30 && num <= 35) {
                // sprintf(identificador,"Cliente %d:",id);
                sprintf(mensaje, "He ido al baño y he perdido el turno asi que me voy");

                pthread_mutex_lock(&colaClientes);
                clientes[posicionCliente].atendido = 4;
                clientes[posicionCliente].id = 0;
                pthread_mutex_unlock(&colaClientes);

                pthread_mutex_lock(&fichero);
                writeLogMessage(identificador, mensaje);
                printf("%s: %s\n", identificador, mensaje);
                pthread_mutex_unlock(&fichero);

                pthread_exit(NULL);
            } else if (num > 35 && num < 45) {
                // sprintf(identificador,"Cliente %d:",id);
                sprintf(mensaje, "Me canse de esperar y me fui");

                pthread_mutex_lock(&colaClientes);
                clientes[posicionCliente].atendido = 4;
                clientes[posicionCliente].id = 0;
                pthread_mutex_unlock(&colaClientes);

                pthread_mutex_lock(&fichero);
                writeLogMessage(identificador, mensaje);
                printf("%s: %s\n", identificador, mensaje);
                pthread_mutex_unlock(&fichero);

                pthread_exit(NULL);
            } else {
                // sprintf(identificador,"Cliente %d:",id);
                sprintf(mensaje, "Estoy esperando en la cola para ser atendido");
                pthread_mutex_lock(&fichero);
                writeLogMessage(identificador, mensaje);
                printf("%s: %s\n", identificador, mensaje);
                pthread_mutex_unlock(&fichero);
                sleep(3);
            }
            
        } else {

            /**
            Busca si hay mquinas libres
            */
            
            maquinasCheckinVariable = 0;
            i = 0;

            pthread_mutex_lock(&maquinas);

            while (maquinasCheckinVariable != 1 && i < numMaquinas) {
            
                if (maquinasCheckin[i] == 0) {
                    maquinasCheckinVariable = 1;
                    maquinasCheckin[i] = 1;
                }

                i++;
            }

            pthread_mutex_unlock(&maquinas);

            if (maquinasCheckinVariable == 1) {
                i--;
                

                pthread_mutex_lock(&colaClientes);
                clientes[posicionCliente].atendido = 2;
                pthread_mutex_unlock(&colaClientes);
                //Hay maquina libre, entonces duerme 6 segundos (simula la atencion)
                sleep(6);

                pthread_mutex_lock(&maquinas);
                maquinasCheckin[i] = 0;
                pthread_mutex_unlock(&maquinas);

                pthread_mutex_lock(&colaClientes);
                clientes[posicionCliente].atendido = 3;
                pthread_mutex_unlock(&colaClientes);

            } else {
                //No hay maquinas, duerme 3 segundos
                sleep(3);

                num = aleatorios(1, 100);
                if (num < 50) {
                    checkin = 0;
                }
            }
        }

        pthread_mutex_lock(&colaClientes);
        atendido = clientes[posicionCliente].atendido;
        pthread_mutex_unlock(&colaClientes);
    }

    /**
    Comprobar si está siendo atendido
    */
    if (atendido == 4) { //TERMINADO de ATENDER
        // sprintf(identificador, "Cliente %d:", id);
        sprintf(mensaje, "Me han terminado de atender");
        pthread_mutex_lock(&fichero);
        writeLogMessage(identificador, mensaje);
        printf("%s: %s\n", identificador, mensaje);
        pthread_mutex_unlock(&fichero);

        pthread_mutex_lock(&colaClientes);
        clientes[posicionCliente].id = 0;
        pthread_mutex_unlock(&colaClientes);

        pthread_exit(0);
    }

    while (atendido == 2) { //SIENDO ATENDIDO
        sleep(2);

        pthread_mutex_lock(&colaClientes);
        atendido = clientes[posicionCliente].atendido;
        pthread_mutex_unlock(&colaClientes);
    }

    num = aleatorios(1, 100);

    pthread_mutex_lock(&finalizar);
    fin = acabar;
    pthread_mutex_unlock(&finalizar);

    int aux = num <= 30;

    pthread_mutex_lock(&ascensor);

    /**
    Miramos si está esperando por el ascensor
    */
    while(aux == 0 && !fin && !estadoAscensor){
        pthread_mutex_unlock(&ascensor);

        sprintf(mensaje, "Esta esperando por el ascensor");
        //printf("Estado ascensor: %d\n", estadoAscensor);
        pthread_mutex_lock(&fichero);
        writeLogMessage(identificador, mensaje);
        printf("%s: %s\n", identificador, mensaje);
        pthread_mutex_unlock(&fichero);

        sleep(3);


        pthread_mutex_lock(&finalizar);
        fin = acabar;
        pthread_mutex_unlock(&finalizar);

    }

    pthread_mutex_unlock(&ascensor);


    /**
    El cliente se fue por las escaleras
    */
    if (fin || aux) {
        sprintf(mensaje, "Me fui para la habitacion por las escaleras");
        pthread_mutex_lock(&fichero);
        writeLogMessage(identificador, mensaje);
        printf("%s: %s\n", identificador, mensaje);
        pthread_mutex_unlock(&fichero);

        pthread_mutex_lock(&colaClientes);
        clientes[posicionCliente].id = 0;
        pthread_mutex_unlock(&colaClientes);

        pthread_exit(0);

    } else {

        pthread_mutex_lock(&colaClientes);
        clientes[posicionCliente].ascensor = 1;
        pthread_mutex_unlock(&colaClientes);
        

        //Se incrementa el NUMERO DE CLIENTES EN EL ASCENSOR
        posAs = ++numClientesAscensor;

        pthread_mutex_unlock(&ascensor);

        // printf(identificador, "Cliente %d:", id);
        sprintf(mensaje, "Esta en el ascensor");
        pthread_mutex_lock(&fichero);
        writeLogMessage(identificador, mensaje);
        printf("%s: %s\n", identificador, mensaje);
        pthread_mutex_unlock(&fichero);

        pthread_mutex_lock(&ascensor);

        if ( posAs >= 6) {

            estadoAscensor = 0;

            pthread_mutex_unlock(&ascensor);

            sleep(6);

            pthread_mutex_lock(&ascensor);

            //Libera su posicion del ascensor
            pthread_cond_signal(&conAscensor[posAs-1]);

        } else {

            pthread_cond_wait(&conAscensor[posAs], &ascensor);
        }

        numClientesAscensor--;


        if (numClientesAscensor == 0) {

            //Es el último, marca que EL ASCENSOR HA DEJADO DE FUNCIONAR

            estadoAscensor = 1;
        }else{
            pthread_cond_signal(&conAscensor[numClientesAscensor]); 
        }

        pthread_mutex_unlock(&ascensor);

        /**
        Se indica que ha dejado el ascensor
        */
        sprintf(identificador, "Cliente %d", id);
        sprintf(mensaje, "Deja el ascensor");
        pthread_mutex_lock(&fichero);
        writeLogMessage(identificador, mensaje);
        printf("%s: %s\n", identificador, mensaje);
        pthread_mutex_unlock(&fichero);
    }

    /**
    Libera su posición en la cola de clientes y se va
    */
    pthread_mutex_lock(&colaClientes);
    clientes[posicionCliente].id = 0;
    pthread_mutex_unlock(&colaClientes);
    pthread_exit(0);
}
// Hilo
void* accionesRecepcionista(void* arg) {
    // Buscar al cliente y mirar si es vip o no vip
    int* recepcionista = (int*)arg;
    int posicion = -1;
    int min = 1;
    int porcentaje; // calculamos numeros aleatorios entre 1 y 100
    int contador = 0;
    srand(time(NULL)); // Inicializamos la semilla
    // Para los log
    char identificador[50];
    char mensaje[200];
    // Condicion variable global
    int fin;
    int cliEsp;
 
    // Punto 1 y 2

    // Protegemos la variable
    pthread_mutex_lock(&finalizar);
    fin = acabar;
    pthread_mutex_unlock(&finalizar);

    cliEsp = sizeClientes();

    while (!(fin && cliEsp == 0)) {
   
        pthread_mutex_lock(&colaClientes);

        min = contClientes + 1;
        posicion = -1;

        for (int i = 0; i < numClientes; i++) {
            if (recepcionista[0] == clientes[i].tipo && clientes[i].atendido == 0) {

                porcentaje = aleatorios(1, 100);
                if (clientes[i].id != 0 && min > clientes[i].id) {
                    min = clientes[i].id;
                    posicion = i;
                }
            }
        }
        if (posicion != -1) { // Hay clientes
            // Hemos inicializado la posicon a -1
            clientes[posicion].atendido = 2; // Acctualizando
        }

        pthread_mutex_unlock(&colaClientes);

        if (posicion == -1) {
           
            sleep(1);
            // Hacemos que duerma 1 segundo ya que no ha econtrado clietnes

        } else {

            sprintf(identificador, "Recepcionista_%d", recepcionista[1]); // Identificador del recepcionista
            sprintf(mensaje, "Comineza la atencion");
            pthread_mutex_lock(&fichero);
            writeLogMessage(identificador, mensaje);
            pthread_mutex_unlock(&fichero);

            /** Se podría poner al inicio del programa, ya que es una de las primeras
                 cosas que hace.
            */
             
            if (porcentaje <= 80) {

                sprintf(mensaje, "El cliente %d tiene todo en regla", clientes[posicion].id);
                sleep(aleatorios(1, 4));

                printf("%s:%s \n", identificador, mensaje);
                pthread_mutex_lock(&fichero);
                writeLogMessage(identificador, mensaje);
                pthread_mutex_unlock(&fichero);

                pthread_mutex_lock(&colaClientes);
                clientes[posicion].atendido = 3; // El cliente ya está atendido
                pthread_mutex_unlock(&colaClientes);

            } else if (porcentaje > 80 && porcentaje <= 90) { // Un 10% de los pacientes

                sprintf(mensaje, "El cliente %d está mal identificado", clientes[posicion].id);

                sleep(aleatorios(2, 6));

                printf("%s: %s \n", identificador, mensaje);
                pthread_mutex_lock(&fichero);
                writeLogMessage(identificador, mensaje);
                pthread_mutex_unlock(&fichero);

                pthread_mutex_lock(&colaClientes);
                clientes[posicion].atendido = 3; // El cliente también ha sido atendido
                pthread_mutex_unlock(&colaClientes);

            } else if (porcentaje > 90) {

                sleep(aleatorios(6, 10));

                sprintf(mensaje, "El cliente %d no presenta el pasapaorte vacunal", clientes[posicion].id);
                pthread_mutex_lock(&fichero);
                writeLogMessage(identificador, mensaje);
                pthread_mutex_unlock(&fichero);

                printf("%s : %s \n", identificador, mensaje);
                pthread_mutex_lock(&colaClientes);
                clientes[posicion].atendido = 4; // Al no tener el pasaporte vacunal debe abandonar el hotel.
                pthread_mutex_unlock(&colaClientes);
            }


            sprintf(mensaje, "Ha finalizado la atención");
            pthread_mutex_lock(&fichero);
            writeLogMessage(identificador, mensaje);
            pthread_mutex_unlock(&fichero);

            /**
             PAUSA para el café cuando el recepcionista haya atendidio a 5 clientes,
             siempre que sea no vip
             */
            if (recepcionista[0] == 0) {
                contador += 1;
         
                if (contador == 5) {
                //Ha atendido a 5 clientes, descansa
                    contador = 0;
                    sleep(5);
                }
            }
        }
        pthread_mutex_lock(&finalizar);
        fin = acabar;
        pthread_mutex_unlock(&finalizar);

        //Los clientes en espera
        cliEsp = sizeClientes();
    }
}

int aleatorios(int min, int max) { // Función para calcular numeros aleatorios
    return rand() % (max - min + 1) + min;
}


    /**
    Fumcion para terminar el proceso 
    Todos deben de haber abandonado el ascensor
    */
void fin() {
    pthread_mutex_lock(&finalizar);
    acabar = 1;
    pthread_mutex_unlock(&finalizar);

    

    pthread_mutex_lock(&ascensor);
    if(numClientesAscensor > 0){

        estadoAscensor = 0;
        
        pthread_mutex_unlock(&ascensor);

        sleep(6);

        pthread_mutex_lock(&ascensor);
        pthread_cond_signal(&conAscensor[numClientesAscensor]);
        
    }

    pthread_mutex_unlock(&ascensor);
    
}

    /**
    Numero de clientes totales que han pasado por el hotel
    */
int sizeClientes() {
    int res = 0;
    pthread_mutex_lock(&colaClientes);
    for (int i = 0; i < numClientes; i++) {
        if (clientes[i].id != 0) {
            res++;
        }
    }
    pthread_mutex_unlock(&colaClientes);
    return res;
}

// Aumentar el numero de clientes o maquinas mientras el programa esta en ejecucion
void aumentar() {
    int num = 0;
    int aumento = 0;
    int tries = 0;
    char mensaje[25];
    do {
        printf("¿Que desea aumentar?\n");
        printf("1)Numero de clientes\n");
        printf("2)Numero de maquinas\n");
        tries++;
        scanf("%d", &num);
    } while (num != 1 && num != 2 && tries < 2);

    if (num == 1 || num == 2) {
        printf("Indique en cuanto: ");
        scanf("%d", &aumento);

        switch (num) {
        case 1:
            /* Redimensionamos el puntero y lo asignamos a un puntero temporal */
            pthread_mutex_lock(&colaClientes);
            struct cliente* tmp_clie;
            tmp_clie = (struct cliente*)realloc(clientes, sizeof(struct cliente) * (aumento + numClientes));

            if (tmp_clie == NULL) {
                // printf("Hubo un error en el aumento de capacidad\n");
                perror("Hubo un error en el aumento de capacidad\n");
            } else {
                /* Reasignación exitosa. Asignar memoria a clientes */
                for(int i = numClientes; i < numClientes+aumento; i++){
                    tmp_clie[i].id = 0;
                    tmp_clie[i].tipo = 0;
                    tmp_clie[i].atendido = 0;
                    tmp_clie[i].ascensor = 0;
                }
                clientes = tmp_clie;
                numClientes += aumento;
            }
            pthread_mutex_unlock(&colaClientes);

            printf("Se aumento correctamente\n");

            pthread_mutex_lock(&fichero);
            sprintf(mensaje, "Aumento de clientes a %d", numClientes);
            writeLogMessage("Main", mensaje);
            pthread_mutex_unlock(&fichero);

            break;
        case 2:
            /* Redimensionamos el puntero y lo asignamos a un puntero temporal */
            pthread_mutex_lock(&maquinas);
            int* tmp_maq;
            tmp_maq = (int*)realloc(maquinasCheckin, sizeof(int) * (aumento + numMaquinas));

            if (tmp_maq == NULL) {
                // printf("Hubo un error en el aumento de capacidad");
                perror("Hubo un error en el aumento de capacidad\n");
            } else {
                /* Reasignación exitosa. Asignar memoria a clientes */
                for(int i = numMaquinas; i < numMaquinas+aumento; i++){
                    tmp_maq[i] = 0;
                }
                maquinasCheckin = tmp_maq;
                numMaquinas += aumento;
            }
            pthread_mutex_unlock(&maquinas);

            printf("Se aumento Correctamente\n");

            pthread_mutex_lock(&fichero);
            sprintf(mensaje, "Aumento de maquinas a %d", numMaquinas);
            writeLogMessage("Main", mensaje);
            pthread_mutex_unlock(&fichero);
            break;
        default:
            printf("Opcion incorrecta\n");
        }
    } else {
        printf("Demasiados intentos incorrectos\n");
    }
}


     /**
    Escribir los logs en el fichero
     */
void writeLogMessage(char* id, char* msg) {
    // Calculamos la hora actual
    time_t now = time(0);
    struct tm* tlocal = localtime(&now);
    char stnow[25];
    strftime(stnow, 25, "%d/%m/%y %H:%M:%S", tlocal);
    // Escribimos en el log
    logFile = fopen(logFileName, "a");
    fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);
    fclose(logFile);
}
