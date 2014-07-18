#ifndef LIBRARYS_H
#define LIBRARYS_H
#include "library.h"
//inizializza socket, bind, listen sul server
int InitServerSocket ( struct sockaddr_in *server, int port, int maxOperatorsQueue );
//gestisce N connessioni contemporanee
void connectionManager ( int sockfd, int opNumber, int kPackages, Package *handler,struct sockaddr_in client, int clientsize );
//thread (vive fin quando il client è connesso)
void *thread_connection_handler ( void *parametri );
//avvia la comunicazione col client. invia i k pacchetti al client
int threadClientInit ( int sockfd, Package *handler, int kPackages );
//gestisce le comunicazioni col client. vive fin quando non riceve un segnale dal client
void talkWithClient ( int client_sock, Package *handler );
//gestisce i comandi ricevuti dal client
int commandSwitchServer ( int command, char *cmdPointer, Package *handler, int sockfd );
//comando elencaserver
void elencaserver_server ( int sockfd, Package *handler );
//comando consegnato
void consegnato_server ( int sockfd, char *cmdPointer, Package *handler );
//comando smista
void smista_server ( int sockfd, char *strbuffer, Package *handler );
//comando ritirato
void ritirato_server ( int sockfd, char *cmdPointer, Package *handler );

#endif
