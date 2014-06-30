#ifndef LIBRARY_H
#define LIBRARY_H

/*librerie standard del c*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*system call base*/
#include <unistd.h>
/*definizioni usate dalle system call*/
#include <fcntl.h>
/*gestione errori e messaggi di errore*/
#include <errno.h>
/* non necessarie ma richieste da alcune distribuzioni unix */
#include <sys/types.h>
#include <sys/stat.h>

/*gestione socket*/
#include <sys/socket.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


typedef enum statoarticoloT {

	STORAGE, //magazzino
	TOBEDELIVERED, //da consegnare
	DELIVERED, // consegnato
	WITHDRAWN // ritirato

} status;

typedef struct PKG {

	char codice_articolo[32];
	char descrizione_articolo[256];
	char indirizzo_destinazione[256];
	status stato_articolo;
	pthread_mutex_t sem;
	struct PKG *next;
} Package;




#endif