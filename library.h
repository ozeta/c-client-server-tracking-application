#ifndef LIBRARY_H
#define LIBRARY_H

/*librerie standard del c*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
/*system call base*/
#include <unistd.h>
/*definizioni usate dalle system call*/
#include <fcntl.h>
/*libreria segnali*/
#include <signal.h>
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
/*gestione thread*/


#define STRING 256
int DEBUG;
typedef enum statoarticoloT {

	STORAGE			= 	0,	// magazzino
	TOBEDELIVERED	= 	1,	// da consegnare
	DELIVERED		= 	2,	// consegnato
	COLLECTED		= 	3 	// ritirato

} Status;

typedef enum commandHashTable {

	ELENCASERVER 	=	0,	
	CONSEGNATO 		=	1,
	RITIRATO		= 	2,	
	SMISTA			= 	3,
	ELENCA 			= 	4

} Hash;

//mutex per la gestione dei thread
pthread_mutex_t 		maxThreadsMutex;
pthread_mutex_t 		packageMutex;

//variabile globale per la gestione dei thread
int 					maxThread;

typedef struct PKG {

	char				codice_articolo[256];
	char				descrizione_articolo[256];
	char				indirizzo_destinazione[256];
	Status 				stato_articolo;
	pthread_mutex_t 	m_lock;
	struct PKG *		next;
} Package;

typedef struct tmpStruct {

	int 				sockfd;
	int 				kPackages;
	Package *			handler;
} Passaggio;



/**controllo dei parametri di ingresso*/
//controlla gli argomenti del server
int serverInputCheck ( int argc, char **argv );
//controlla che l'argomento sia un numero positivo
int checkArguments ( char *argument, int argNum );
//genera numeri interi casuali
int randomIntGenerator ( int inf, int sup );
//verifica che una stringa sia un indirizzo ip valido
int isValidIpAddress ( char *ipAddress );
//verifica che la porta sia valida
int isPortValid ( char *argument, int inf, int sup );
//controlla gli argomenti del client
void clientInputCheck ( int argc, char **argv );
/***/

//mostra il menu
void showMenu ( void );

/**gestione della lista locale*/
//stampa dell'intera lista, ricorsiva
void pkglist_print_r ( Package *handler );
void pkglist_sort_print_r ( Package *handler);
//stampa singolo pacchetto
void pkg_print ( Package *handler );
//alloca e inizializza/riempie pacchetto
Package * pkg_initialize ( char **buffer, int status );
//aggiunge in coda un pacchetto
Package * pkg_enqueue_r ( Package * handler, char **buffer, int status );
//aggiunge in testa
Package * pkg_push_r ( Package * handler, char **buffer, int status );
//cancella un pacchetto dalla lista
Package * pkg_delete_r ( Package * handler, char *buffer0 );
//cancella tutta la lista
void list_delete_r ( Package * handler );
//scarica su fd il contenuto della memoria
void package_dump ( Package *handler, int outFD, int print ) ;
//cancella la memoria
void list_dump ( Package * handler, int outFD, int print);
//ricerca pacchetto
Package * pkg_find_r ( Package * handler, char *pkgCode );
//ricerca pacchetto, bloccando il mutex
Package * pkg_find_mutex ( Package * handler, char *pkgCode );
//aggiunge in coda alla lista, bloccando il mutex
Package * pkg_enqueue_r_mutex ( Package * handler, char **buffer, int status );
//recupera un pacchetto in base al suo stato
Package * getStoredPackage_r ( Package * handler, int status );
//legge da file descriptor e crea una lista concatenata
Package * createList ( Package *handler, int inputFD, int tokensNumber, int status, int print );

/**funzioni di lettura e analisi*/
//legge da fd fino a raggiungere '\n'. salva in strbuffer il contenuto del fd.
int readLine ( int inputFD, char *strbuffer );
//legge da strbuffer e separa i token salvandoli in un array di stringhe
void getTokens ( char *string[], char *strbuffer, int tokensNumber );
//legge la stringa fino ad un carattere terminale e restituisce la posizione
char *getSubstr ( char *result, char *input, char terminal );


//legge da tastiera ed analizza la stringa
int getCommand ( int inputFD, char **commandString );
//legge la stringa strbuffer e ne estrae il comando salvandolo in string
void splitCommand ( char *string, const char *strbuffer );
//converte un comando testuale in numero
int commandToHash ( char *command );

//codifica un elemento della lista per la trasmissione
char *encodePkgForTransmission ( Package *handler );
//decodifica un pacchetto ricevuto
char *decodePkgfromTransmission ( char *strbuffer );
//inizializza array di stringhe
void memsetString ( char **str, int tokensNumber );
//alloca stringa dinamicamente
char * stringMalloc ( void );
//libera lo spazio dell'array
void freeArray ( char **str, int tokensNumber );
//write su socket + gestione eventuale errore
int sendMessage ( int sockfd, char *message );
//decodifica lo stato articolo da enum a stringa
void decodeStatus (char *status, Status stat);



#endif
