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
	if (argc!=2&&argc!=4){
		printf("Argumentos incorrctos\n");
		exit(EXIT_FAILURE);
	}
	
	//Convertimos la direccion obtenida a network byte order
	struct in_addr addr;
	inet_aton(argv[1], &addr);

	if (argc==2){
		serv=getservbyname("qotd","udp");
		puerto = serv->s_port;
	}
	
	if (argc==4){
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
		perror("socket");
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
	
	//Enlazamos el socket y comprobamos

	int errBind;
	errBind = bind(sock, (struct sockaddr *) &cliente, sizeof(cliente));
	if (errBind == -1){
		printf("Error al enlazar");
		perror("bind()");
		exit(EXIT_FAILURE);

	//Realizamos el envio y comprobamos que ha ido bien
	}
	int errSend;
	char *mensaje = "Enviame algo";
	int len = strlen(mensaje);
	errSend = sendto (sock, mensaje, len, 0, (struct sockaddr*) &servidor,sizeof(servidor));
	if (errSend==-1){
		printf("Error al enviar\n");
		perror("sendto()");
		exit(EXIT_FAILURE);
	}

	//Realizamos la recepcion y comprobamos que ha ido bien

	socklen_t lenght = sizeof(cliente);
	int errRecv;

	errRecv =recvfrom (sock,buffer, sizeof(buffer),0, (struct sockaddr*) &cliente,&lenght);
	
	if (errRecv==-1){
		printf("Error al recibir\n");
		perror("recvfrom()");
		exit(EXIT_FAILURE);
	}
	printf("%s", buffer);
	return 0;
}
