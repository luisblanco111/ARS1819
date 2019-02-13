// Practica tema 8, Blanco De la Cruz Luis


#include "ip-icmp-ping.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char* argv[]){

	int flag=0;

	//Comprobamos numero de argumentos
	if (argc!=2 && argc!=3){
		printf("Error, numero de argumentos no valido");
		exit(EXIT_FAILURE);
	}
	//Comprobamos si el usuario quiere que le muestre el proceso
	if (argc==3 && strcmp(argv[2], "-v")==0){
		flag=1;
	}

	//Convertimos la ip introducida
	struct in_addr addr;
	inet_aton(argv[1], &addr);
	

	//Creamos el datagrama ICMP
	ECHORequest echoRequest;

	echoRequest.icmpHeader.Type=8;
	echoRequest.icmpHeader.Code=0;
	echoRequest.icmpHeader.Checksum=0;
	echoRequest.ID=getpid();
	echoRequest.SeqNumber=0;
	strcpy(echoRequest.payload, "Payload de ejemplo");


	//Calculamos el checksum
	int numShorts=sizeof(echoRequest)/2;
	unsigned short int *puntero;
	unsigned int acum=0;
	puntero = (unsigned short int *) &echoRequest;
	int i;
	for(i=0; i<numShorts; i++){
		acum = acum+(unsigned int) *puntero;
		puntero++;
	}
	acum = (acum >> 16) + (acum & 0x0000ffff);
	acum = (acum >> 16) + (acum & 0x0000ffff);
	acum = ~acum;
	echoRequest.icmpHeader.Checksum=(unsigned short int) acum;

	//Creamos la estrucutra
	struct sockaddr_in destino;
	destino.sin_family=AF_INET;
	destino.sin_port=0;
	destino.sin_addr=addr;
	socklen_t lengthS = sizeof(destino);

	//Creacion del socket
	int sock;
	sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock==-1){
		printf("Error al crear el socket\n");
		perror("socket()");
		exit(EXIT_FAILURE);
	}

	//Detalles del envio
	if (flag==1){
		printf("Generando cabecera ICMP.\n");
		printf("->Type: %i\n", echoRequest.icmpHeader.Type);
		printf("->Code: %i\n", echoRequest.icmpHeader.Code);
		printf("->Identifier (pid): %i.\n", echoRequest.ID);
		printf("->Seq. number: %i\n", echoRequest.SeqNumber);
		printf("->Cadena a enviar: %s.\n", echoRequest.payload);
		printf("->Checksum: 0x%x.\n", echoRequest.icmpHeader.Checksum);
		printf("->Tamaño total del paquete ICMP :%lu.\n",sizeof(echoRequest));
	}

	//Enviamos la solicitud
	int errSend=sendto(sock, &echoRequest, sizeof(echoRequest), 0, (struct sockaddr *) &destino, sizeof(destino));
	if (errSend == -1){
		printf("Error al enviar\n");
		perror("sendto()");
		exit(EXIT_FAILURE);
	}
	printf("Paquete ICMP enviado a %s. \n", argv[1]);

	ECHOResponse echoResponse;
	int errRecv=recvfrom(sock, &echoResponse, sizeof(echoResponse), 0, (struct sockaddr*)&destino, &lengthS);
	if (errRecv==-1){
		printf("Error al recibir\n");
		perror("recvfrom()");
		exit(EXIT_FAILURE);
	}
	printf("Respuesta recibida desde %s\n", inet_ntoa(destino.sin_addr));
	//Mostramos todas las respuestas posibles
	if (flag==1){
		printf("->Tamaño de la respuesta: %d\n", errRecv);
		printf("->Cadena recibida: %s.\n", echoResponse.payload);
		printf("->Identifier(pid): %u.\n", echoResponse.ID);
		printf("->TTL: %u.\n", echoResponse.ipHeader.TTL);
	}	
	
	switch(echoResponse.icmpHeader.Type){
		case 0:
			printf("Descripcion de la respuesta: respuesta correcta (type 0, code 0)\n");
			break;
		case 3:
			switch(echoResponse.icmpHeader.Code){
				case 0:
					printf("Destination network unreachable (type 3, code 0)\n");
					break;
				case 1:
					printf("Host unreacheble (type 3, code 1)\n");
					break;
				case 2:	
					printf("Protocol unreachable (type 3, code 2)\n");
					break;
				case 3:
					printf("Destination port unreachable (type 3, code 3)\n");
					break;
				case 4:
					printf("Fragmentation required (type 3, code 4)\n");
					break;
				case 5:
					printf("Source route failed (type 3, code 5)\n");
					break;
				case 6:
					printf("Destination network unknown (type 3, code 6)\n");
					break;
				case 7:
					printf("Destination host unknown (type 3, code 7)\n");
					break;
				case 8:
					printf("Source host isolated (type 3, code 8)\n");
					break;
				case 9:
					printf("Network administratively prohibited (type 3, code 9)\n");
					break;
				case 10:
					printf("Host administratively prohibited (type 3, code 10)\n");
					break;
				case 11:
					printf("Network unreachable for ToS (type 3, code 11)\n");
					break;
				case 12:
					printf("Host unreachable for ToS (type 3, code 12)\n");
					break;
				case 13:
					printf("Communication administratevely prohibited (type 3, code 13)\n");
					break;
				case 14:
					printf("Host precedence violation (type 3, code 14)\n");
					break;
				case 15:
					printf("Precedence cutoff in effect (type 3, code 15)\n");
					break;
			}
			break;
		case 5:
			switch(echoResponse.icmpHeader.Code){
				
				case 0:				
					printf("Redirect Datagram for the Network (type 5, code 1)\n");
					break;
				case 1:
					printf("Redirect Datagram for the Host (type 5, code 1)\n");
					break;
				case 2:
					printf("Redirect Datagram for the ToS & network (type 5, code 2)\n");
					break;
				case 3:
					printf("Redirect Datagram for the ToS & (type 5, code 3)\n");
					break;
			}
			break;
		case 8:
			printf("Echo Request (type 8, code 0)\n");
			break;
		case 9:
			printf("Router Advertisement (type 9, code 0)\n");
			break;
		case 10:
			printf("Router Solicitation (type 10, code 0)\n");
			break;
		case 11:
			switch(echoResponse.icmpHeader.Code){
				case 0:
					printf("TTL expired in transit (type 11, code 0)");
					break;
				case 1:
					printf("Fragment reassembly time exceeded (type 11, code 1)");
		       			break;
			}
			break;
		case 12:
			switch(echoResponse.icmpHeader.Code){
				case 0:
					printf("Pointer indicates the error (type 12, code 0)");
					break;
				case 1:
					printf("Missing a required option (type 12, code 1)");
					break;
				case 2:
					printf("Bad lenght (type 12, code 2)");
					break;
			}
		case 13:
			printf("Timestamp (type 13, code 0)");
			break;
		case 14:
			printf("Timestamp reply (type 14, code 0)");
			break;
		case 40:
			printf("Photuris, Security Failure (type 40)");
			break;

	}


	return 0;	

}
