#ifndef LIBRARYC_H
#define LIBRARYC_H
#include "library.h"
//inizializza la lista locale
Package *initClient ( int sockfd, Package *handler );
//avvia socket, bind, connect
int initClientSocket ( char **argv );
//gestisce i comandi disponibili
Package *commandSwitch ( int command, char *cmdPointer, Package *handler, int sockfd );
//comando elencaserver
void elencaserver_client ( int sockfd, char *commandLine );
//controlla che l'input da tastiera sia valido
int checkCommandInput ( char *strbuffer, int parameters );
//comando consegnato
Package *consegnato_client ( int sockfd, char *strbuffer, Package *handler );
//comando smista
Package *smista_client ( int sockfd, char *strbuffer, Package *handler );
//comando ritirato
void ritirato_client ( int sockfd, char *strbuffer, Package *handler );

#endif
