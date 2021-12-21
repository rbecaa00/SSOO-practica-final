#include <stdio.h>	
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

void nuevoCliente();
void accionesCliente();
void accionesRecepcionista();
void writeLogMessage(char *id, char *msg);

int main(int argc, char* argv[]){
	//**
	//Esto lo hare Rubén Bécares Álvarez
	return 0;
}

void nuevoCliente(){
	cipote blanco
}


void accionesCliente(){
	//Victor escribe aqui
}

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
