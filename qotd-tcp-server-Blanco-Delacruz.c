// Practica tema 6, Blanco de la Cruz Luis

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>

#define MAXLENGTH 512

int sockServer, sockClient;

//Funcion que se encarga de cerrar los puertos una vez introducico controlC por tecaldo
void signal_handler(int signal){
	//Cerramos el socket y comprobamos
	if (shutdown(sockServer, SHUT_RDWR)==-1){
		perror("sutdown()");
		exit(EXIT_FAILURE);
	}
	exit(0);
}



int main (int argc, char *argv[]){
	
	signal(SIGINT, signal_handler);

	int puerto=0;
	char buffer[MAXLENGTH];
	struct servent *serv;
	pid_t pid;

	
	switch(argc){

		case 1:
		//Si el usuario no introduce puerto, el servidor se ejecuta en el puesto estandar

			serv=getservbyname("qotd","tcp");
			//Comprobamos la obtencion de puerto
			if (serv==NULL){
				printf("No se ha encontrado el puerto\n");
				perror("getservbyname()");
				exit(EXIT_FAILURE);
			}	
			puerto = serv->s_port;
			break;

		case 3:
			//Si el usuario introduce el puerto, comprobamos que el usuario lo hace correctamente 
			if (strcmp(argv[1], "-p") !=0){
				printf("Error, introduzca la opcion -p para adjuntar puerto\n");
				exit(EXIT_FAILURE);
			}
		
			//Si el numero de puerto es correcto, lo pasamos a network byte order
			puerto = htons(atoi(argv[2]));
			break;
		
		default:
			//Si el usuario introduce los argumentos mal, le indicamos el comando correcto
			printf("El comando correcto es: ./qotd [-p puerto] \n");
			exit(EXIT_FAILURE);
	}

	//Creamos la estructura del servidor
	
	struct sockaddr_in servidor;
	servidor.sin_family = AF_INET;
	servidor.sin_port = puerto;
	servidor.sin_addr.s_addr = htonl(INADDR_ANY);


	//Creamos el socket y comprobamos

	sockServer = socket(AF_INET, SOCK_STREAM,0);
	if (sockServer==-1){
		printf("Error al crear el socket\n");
		perror("socket()");
		exit(EXIT_FAILURE);
	}


	//Enlazamos el socket y comprobamos
	int errBind;
	errBind = bind(sockServer, (struct sockaddr *) &servidor, sizeof(servidor));
	if (errBind ==-1){
		printf("Error al enlazar\n");
		perror("bind()");
		exit(EXIT_FAILURE);
	}
	
	//Empezamos la escucha y comprobamos
	int errListen;
	errListen = listen(sockServer, 10);
	if (errListen == -1){
		printf("Error al escuchar\n");
		perror("listen()");
		exit(EXIT_FAILURE);
	}


	//Creamos estructura para alojar los clientes que pidan
	struct sockaddr_in cliente;
	socklen_t lenght = sizeof(cliente);


	//Bucle de funcionamiento del servidor
	for(;;){
	
		//Recibimos peticiones de los clientes y comprobamos 
		sockClient = accept(sockServer,(struct sockaddr *)&cliente, &lenght);
		if (sockClient ==-1){
			printf("Error al recibir\n");
			perror("accept()");
			exit(EXIT_FAILURE);
		}
		pid = fork();

		//Si es un hijo, tratamos la peticion
		if (pid==0){
			
			//Metemos cabecera de la cita en el buffer
			strcpy(buffer, "Quoute Of The Day from vm2536:\n");
			//Obtenemos la cita e introducimos un final de cadena
			system("/usr/games/fortune -s > /tmp/tt.txt");
			FILE *fich = fopen("/tmp/tt.txt","r");
			int nc = strlen(buffer);
			do{
			buffer[nc] = getc(fich);
			nc++;
			} while ( (feof(fich)==0) && nc < MAXLENGTH);
			fclose(fich);
			buffer[nc-1] = '\0';


			//Enviamos la cita al cliente y comprobamos
			int errSend;
			errSend = send(sockClient, buffer, MAXLENGTH, 0);
			if (errSend == -1) {
				printf("Error al enviar\n");
				perror("sendto()");
				exit(EXIT_FAILURE);
			}
			//Cerramos el socket y dejamos de enviar mensajes
			close(sockClient);
			exit(0);
		}else{
			//Si por el contrario es el padre, no hacemos nada
		}
	}
	return 0;
}
