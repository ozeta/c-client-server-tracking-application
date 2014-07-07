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

int DEBUG; 
//mutex per la gestione dei thread
pthread_mutex_t maxThreadsMutex;
pthread_mutex_t packageMutex;

//variabile globale per la gestione dei thread
int maxThread;


typedef enum statoarticoloT {

	STORAGE			= 0,	// magazzino
	TOBEDELIVERED	= 1,	// da consegnare
	DELIVERED		= 2,	// consegnato
	COLLECTED		= 3 	// ritirato

} Status;

typedef enum commandHashTable {

	ELENCASERVER 	= 0,	
	CONSEGNATO 		= 1,
	RITIRATO		= 2,	
	SMISTA			= 3,
	ELENCA 			= 4

} Hash;

typedef struct PKG {

	char codice_articolo[32];
	char descrizione_articolo[256];
	char indirizzo_destinazione[256];
	Status stato_articolo;
	pthread_mutex_t m_lock;
	struct PKG *next;
} Package;

typedef struct tmpStruct {

	int sockfd;
	int kPackages;
	Package *handler;


} Passaggio;



void pkglist_print (Package *handler);
void pkg_print (Package *handler);
Package * pkg_initialize (char **buffer, int status);
Package * pkg_enqueue (Package * handler, char **buffer, int status);
Package * pkg_push (Package * handler, char **buffer, int status);
Package * pkg_delete (Package * handler, char *buffer0);
Package * pkg_find (Package * handler, char *pkgCode);
Package * getStoredPackage (Package * handler, int status);
int isEndOfMessage (char *string);
Package * createList (Package *handler, int inputFD, int tokensNumber, int status);
int readLine (int inputFD, char *strbuffer);
void getTokens (char *string[], char *strbuffer, int tokensNumber);
char *getSubstr (char *result, char *input, char terminal, int stepup);
int checkArguments (char *argument, int argNum);
int serverInputCheck (int argc, char **argv);
int randomIntGenerator (int inf, int sup);
int isValidIpAddress (char *ipAddress);
int isPortValid (char *argument, int inf, int sup);
void clientInputCheck (int argc, char **argv);
void showMenu (void);
void commandSwitch (int command, char *cmdPointer, Package *handler, int sockfd);
void commandSwitchServer (int command, char *cmdPointer, Package *handler, int sockfd);
int commandToHash (char *command);
void getCommand (char *string, const char *strbuffer);
int getLineA (int inputFD, char *cmdPointer, char **test);
int getLine (int inputFD, char **cmdPointer);
int InitServerSocket (struct sockaddr_in *server, int port, int maxOperatorsQueue);
int initClientSocket (char **argv);
char *encodePkgForTransmission (Package *handler);
void threadClientInit (int sockfd, Package *handler, int kPackages);
void *thread_connection_handler (void *parametri);
void talkWithClient (int client_sock, Package *handler);
void elencaserver_client (int sockfd, char *cmdPointer);
void elencaserver_server (int sockfd, Package *handler);
void commandSwitch (int command, char *cmdPointer, Package *handler, int sockfd);
#endif
