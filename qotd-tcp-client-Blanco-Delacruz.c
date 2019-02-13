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

#define MAXLENGTH 512


int main (int argc, char *argv[]){
	
	int puerto=0;
	char buffer[MAXLENGTH];
	struct servent *serv;
	int sock;
	struct in_addr addr;
	
	switch(argc){
		case 2:
			//Comprobamos la ip y la convertimos	
			if (inet_aton(argv[1], &addr) == 0){
				printf("Direccion IP introducida no valida. \n");
				exit(EXIT_FAILURE);
			}


			//El usuario no introduce el puerto, se toma el dado por defecto
			serv=getservbyname("qotd","tcp");
			puerto = serv->s_port;
			break;

		case 4:
			//Comprobamos la ip y la convertimos	
			if (inet_aton(argv[1], &addr) == 0){
				printf("Direccion IP introducida no valida. \n");
				exit(EXIT_FAILURE);
			}

			//Si el usuario introduce el puerto, comprobamos que lo hace correctamente
			if (strcmp(argv[2], "-p") !=0){
				printf("Error, introduzca la opcion -p para adjuntar puerto");
				exit(EXIT_FAILURE);
			}
			//Convertimos numero de puerto introducido para poder utilizarlo mas adelante
		
			puerto = htons(atoi(argv[3]));
			break;

		default:
			//Si el usuario introduce los argumentos mal, le indicamos el comando correcto
				printf("El comando correcto es ./qotd ip [-p puerto]\n");
				exit(EXIT_FAILURE);
		}


	//Creamos estrucutras del cliente y del servidor
	struct sockaddr_in cliente;
	cliente.sin_family = AF_INET;
	cliente.sin_port = 0;
	cliente.sin_addr.s_addr = INADDR_ANY;	

	struct sockaddr_in servidor;
	servidor.sin_family = AF_INET;
	servidor.sin_port = puerto;
	servidor.sin_addr= addr;


	//Creamos el socket y comprobamos
	sock = socket(AF_INET, SOCK_STREAM,0);
	if (sock==-1){
		printf("Error al crear el socket\n");
		perror("socket");
		exit(EXIT_FAILURE);
	}


	//Enlazamos el socket y comprobamos

	int errBind;
	errBind = bind(sock, (struct sockaddr *) &cliente, sizeof(cliente));
	if (errBind == -1){
		printf("Error al enlazar");
		perror("bind()");
		exit(EXIT_FAILURE);

	}
	//Realizamos la conexion y comprobamos que ha ido bien
	int errConn;
	errConn = connect(sock,(struct sockaddr*) &servidor,sizeof(servidor));
	if (errConn==-1){
		printf("Error al enviar\n");
		perror("connect()");
		exit(EXIT_FAILURE);
	}

	//Realizamos la recepcion y comprobamos que ha ido bien
	if (recv(sock,buffer, MAXLENGTH,0)==-1){
		printf("Error al recibir\n");
		perror("recvfrom()");
		exit(EXIT_FAILURE);
	}

	//Imprimimos la cita recibida
	printf("%s", buffer);	

	//Comenzamos el cierre de conexion y las comprobaciones
	int errShut;
	errShut = shutdown(sock, SHUT_RDWR);
	if (errShut == -1){
		printf("Error al cerrar conexion");
		perror("shutdown()");
		exit(EXIT_FAILURE);
	} 
	if (recv(sock,buffer, MAXLENGTH,0)!=0){

		printf("Error al cerrar conexcion");
		perror("recv()");
		exit(EXIT_FAILURE);
	}
	close(sock);
	return 0;
}
