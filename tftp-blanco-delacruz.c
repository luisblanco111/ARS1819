// Practica tema 7, Blanco De la Cruz Luis

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

int main (int argc, char* argv[]){
int puerto =0;
char datos[516];
int flag, numBloque;

//Comprobamos que los argumentos estan bien introducidos
if (argc!=4 && argc!=5){
	printf("Argumentos mal introducidos\n");
	exit(EXIT_FAILURE);
}

//Comprobamos si el usuario quiere que le muestre el proceso
if (argc==5 && strcmp(argv[4], "-v")==0){
	flag = 1;
}

//Convertimos la ip introducida
struct in_addr addr;
inet_aton(argv[1], &addr);

//Obtencion del puerto en el que funciona tftp
struct servent *serv;
serv = getservbyname("tftp","udp");
puerto = serv->s_port;

//Creacion del socket
int sock;
sock=socket(AF_INET, SOCK_DGRAM,0);
if (sock==-1){
	printf("Error al crear el socket\n");
	perror("socket()");
	exit(EXIT_FAILURE);
}

//Creamos estructura del servidor y del cliente
struct sockaddr_in servidor;
servidor.sin_family=AF_INET;
servidor.sin_port=puerto;
servidor.sin_addr=addr;
socklen_t lengthS=sizeof(servidor);

struct sockaddr_in cliente;
cliente.sin_family=AF_INET;
cliente.sin_port=0;
cliente.sin_addr.s_addr=INADDR_ANY;

//Enlazamos socket y comprobamos
int errBind;
errBind = bind(sock, (struct sockaddr *) &cliente, sizeof(cliente));
if (errBind == -1){
	printf("Error al enlazar\n");
	perror("bind()");
	exit(EXIT_FAILURE);
}

if (strcmp(argv[2], "-r")==0){

	//Creamos el RRQ y metemos algo, ya que strcat no funciona bien si lo ultimo que hay es un 0 
	char RRQ[128] = "OP";
	//Metemos el nombre del archivo en el RRQ
	strcat(RRQ, argv[3]);
	//Como pasa antes, hay que poner algo para que funcione strcat y luego metemos el modo
	RRQ[2+strlen(argv[3])] = 100;
	strcat(RRQ,"octet");
	RRQ[2+strlen(argv[3])] = 0;
	RRQ[0]=0;
	RRQ[1]=1;

	//Enviamos RRQ y comprobamos
	int errRRQ;
	errRRQ = sendto(sock, RRQ, 256, 0, (struct sockaddr *) &servidor, sizeof(servidor));
	if (errRRQ ==-1){
		printf("Error al enviar\n");
		perror("sendto()");
		exit(EXIT_FAILURE);
	}
	if (flag==1){
		printf("Enviada RRQ de fichero %s a servidor %s\n", argv[3], argv[1]);
	}

	FILE *fichero = fopen(argv[3], "w");
	int bloqueR;
	int bloqueE=0;
	do{
		//Recibo un paquete de datos y compruebo que es el bloque correcto
		bloqueR = recvfrom(sock, datos, 516, 0, (struct sockaddr *) &servidor, &lengthS);
		if (bloqueR == -1){
			printf("Error al recibir\n");
			perror("recvfrom()");
			exit(EXIT_FAILURE);
		}
		numBloque=(unsigned char)datos[2]*256+(unsigned char)datos[3];
		bloqueE++;

		if (bloqueE!=numBloque){
			printf("Paquetes desordenados\n");
			exit(EXIT_FAILURE);
		}

		
		//Comprobamos si hemos recibido algun error
		if (datos[1]==5){
			printf("Error: %i%i\n", datos[2], datos[3]);
			exit(EXIT_FAILURE);
		}
		if (flag==1){
		printf("Recibido bloque numero %i\n",numBloque);
		}

		//Escribimos lo recibido en el fichero
		fwrite(datos+4, 1, bloqueR-4, fichero);

		
		//Enviamos ACK usando el mismo buffer de datos y comprobamos envio
		datos[1]=4;
		int errACK;
		errACK = sendto(sock, datos, 4, 0, (struct sockaddr *)&servidor, sizeof(servidor));
		if (errACK ==-1){
			printf("Error al enviar ACK");
			perror("sendto()");
			exit(EXIT_FAILURE);
		}
		if (flag==1){
		printf("ACK del bloque %i enviado\n", numBloque);
	}


	}while(bloqueR==516);
	
	if (flag==1){
		printf("El ultimo bloque recibido fue el %i\n", numBloque);
	}


	fclose(fichero);
}

if (strcmp(argv[2], "-w")==0){

	//Creamos el WRQ y metemos algo, ya que strcat no funciona bien si lo ultimo que hay es un 0 
	char WRQ[128] = "OP";
	//Metemos el nombre del archivo en el WRQ
	strcat(WRQ, argv[3]);
	//Como pasa antes, hay que poner algo para que funcione strcat y luego metemos el modo
	WRQ[2+strlen(argv[3])] = 100;
	strcat(WRQ,"octet");
	WRQ[2+strlen(argv[3])] = 0;
	WRQ[0]=0;
	WRQ[1]=2;

	//Enviamos WRQ y comprobamos
	int errWRQ;
	errWRQ = sendto(sock, WRQ, 128, 0, (struct sockaddr *) &servidor, sizeof(servidor));
	if (errWRQ ==-1){
		printf("Error al enviar\n");
		perror("sendto()");
		exit(EXIT_FAILURE);
	}
	if (flag==1){
		printf("Enviada WRQ de fichero %s a servidor %s\n", argv[3], argv[1]);
	}

	FILE *fichero = fopen(argv[3], "r");
	int posicionFich;

	while (!feof(fichero)){

	//Leemos el fichero hasta rellenar 512 bytes
	datos[posicionFich%512+4] =fgetc(fichero);
	posicionFich++;

	//Si ha llegado al final del fichero o a 512 bytes leidos recibimos un ack
	if (feof(fichero) || posicionFich%512==0){
		recvfrom(sock, datos, 4, 0, (struct sockaddr *)&servidor, &lengthS);

		//Calculamos numero de bloque
		numBloque=(unsigned char)datos[2]*256+(unsigned char)datos[3];
		
		//Comprobamos si hemos recibido algun error
		if (datos[1]==5){
			printf("Error: %i%i\n", datos[2], datos[3]);
			exit(EXIT_FAILURE);
		}

		if (flag==1){
		printf("Recibido ack numero %i\n",numBloque);
		}

		//Calculamos el tamaño del paquete a enviar
		int enviarT;
		if (posicionFich%512==0){
			enviarT=516;
		}
		else{
		enviarT=posicionFich%512+3;
		}	
		numBloque++;
		datos[1]=3;
		datos[2]=numBloque/256;
		datos[3]=numBloque%256;
		int errSend;
		errSend = sendto(sock, datos, enviarT, 0, (struct sockaddr*)&servidor, sizeof(servidor));
		if (errSend ==-1){
			printf("Error al enviar");
			perror("sendto()");
			exit(EXIT_FAILURE);
		}

		if (flag==1){
		printf("Bloque numero %i enviado\n", numBloque);
		}	
	}

	}

	if (flag==1){
	printf("El ultimo bloque recibido fue el %i\n", numBloque);
	}

	fclose(fichero);
}

return 0;
}
