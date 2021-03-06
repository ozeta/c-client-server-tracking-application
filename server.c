/*=============================================================================
  Nome: server.c
  Autori:
	MARCO CARROZZO     	N86/1240
	MAURIZIO DEL PRETE 	N86/783

	Progetto: Corriere Espresso
  ===========================================================================*/

/*
	compilazione:
	gcc -pthread server.c library.c library_server.c -o server
*/

#include "library_server.h"

int main ( int argc , char *argv[] ) {
	//check preliminare su input
	//signal( SIGINT,SIG_IGN );
	Status 			status 				= STORAGE;
	Package *		handler				= NULL;
	int 			inputFD 			= serverInputCheck ( argc, argv );					//controlla i parametri di input
	int 			opNumber 			= atoi ( argv[1] );									//converte i parametri in numeri
	int 			kPackages			= atoi ( argv[2] );
	int 			tokensNumber		= 3;	
	int 			maxOperatorsQueue 	= 128;
	int 			err;
	int 			sockfd;
	int 			clientsize;

	if ( ( err = pthread_mutex_init ( &maxThreadsMutex, NULL ) ) != 0 ) 					//inizializza i mutex che gestiscono i thread
		perror ( "Impossibile allocare il mutex per i thread" ), exit ( -1 );

	handler = ( Package *) createList ( handler, inputFD, tokensNumber, status, 0 );		//legge il file di testo e crea la lista di pacchetti
	close ( inputFD );
	//pkglist_print_r ( handler );
	//return 0;
	struct sockaddr_in *server , client;
	//porta tcp random nel range assegnato da IANA per l'utente
	//srand ( time ( NULL ) );
	//int port = randomIntGenerator ( 1024, 49150 );
	//int port = randomIntGenerator ( 1030, 1039 );
	int port = 44444;
	sockfd = InitServerSocket ( server, port, maxOperatorsQueue );							//inizializza socket e connessione

	connectionManager ( sockfd, opNumber, kPackages, handler, client, clientsize );			//gestisce le connessioni

	close ( sockfd );																		//chiusura di fd e mutex
	pthread_mutex_destroy ( &maxThreadsMutex );

	return 0;
}
