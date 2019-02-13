// Practica tema 5, Blanco de la Cruz Luis

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

#define MAXLENGTH 512


int main (int argc, char *argv[]){
	
	int puerto=0;
	char buffer[MAXLENGTH];
	struct servent *serv;
	int sock;

	//Comprobamos que el usuario introduce los argumentos correctos
	if (argc!=1&&argc!=3){
		printf("Argumentos incorrctos\n");
		exit(EXIT_FAILURE);
	}
	
	//Si el usuario no introduce el numero de puerto lo pregutamos

	if (argc==1){
		serv=getservbyname("qotd","udp");
		if (serv==NULL){
			printf("No se ha encontrado el puerto");
			perror("getservbyname()");
			exit(EXIT_FAILURE);
		}	
		puerto = serv->s_port;
	}
	//Si introduce el puerto, comprobamos que lo ha hecho bien

	if (argc==3){
		int errPuerto = strcmp("-p", argv[2]);
		if (errPuerto !=0){
			printf("Error, introduzca la opcion -p para adjuntar puerto");
			exit(EXIT_FAILURE);
		}
		//Convertimos numero de puerto introducido para poder utilizarlo mas adelante
		
		puerto = htons(atoi(argv[3]));


	}


	//Creamos el socket y comprobamos

	sock = socket(AF_INET, SOCK_DGRAM,0);
	if (sock==-1){
		printf("Error al crear el socket\n");
		perror("socket()");
		exit(EXIT_FAILURE);
	}

	//Creamos la estructura del servidor
	
	struct sockaddr_in servidor;
	servidor.sin_family = AF_INET;
	servidor.sin_port = puerto;
	servidor.sin_addr.s_addr = htonl(INADDR_ANY);


	//Enlazamos el socket y comprobamos
	int errBind;
	errBind = bind(sock, (struct sockaddr *) &servidor, sizeof(servidor));
	if (errBind ==-1){
		printf("Error al enlazar");
		perror("bind()");
		exit(EXIT_FAILURE);
	}
	
	
	//Bucle de funcionamiento del servidor
	for(;;){
	
		//Creamos estructura para alojar los clientes que pidan
		struct sockaddr_in cliente;
		socklen_t lenght = sizeof(cliente);

		//Recibimos peticiones de los clientes y comprobamos 
		int errRecv;
		errRecv = recvfrom(sock, buffer, MAXLENGTH, 0,(struct sockaddr *)&cliente, &lenght);
		if (errRecv ==-1){
			printf("Error al recibir");
			perror("recvfrom()");
			exit(EXIT_FAILURE);
		}
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

		//Enviamos la cita al cliente
		int errSend;
		errSend = sendto(sock, buffer, MAXLENGTH, 0, (struct sockaddr *) &cliente, sizeof(cliente));
		if (errSend == -1) {
			printf("Error al enviar");
			perror("sendto()");
			exit(EXIT_FAILURE);
		}
		
	}
}
