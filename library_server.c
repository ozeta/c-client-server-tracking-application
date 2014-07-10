#include "library_server.h"
#include <pthread.h>


/*inizializza il socket di comunicazione del server*/
int InitServerSocket ( struct sockaddr_in *server, int port, int maxOperatorsQueue ) {

	//creo il socket
	int sockfd = socket ( AF_INET , SOCK_STREAM , 0 );
	char mess[] = "Socket creato...\n";
	if ( sockfd == -1 )
	{
		char mess[] = "Non è possibile creare il socket";
		write ( STDOUT_FILENO, mess, strlen ( mess ) );
	}
	write ( STDOUT_FILENO, mess, strlen ( mess ) );
	
	//preaparo la struttura sockaddr_in che contenga le informazioni di connessione
	server = ( struct sockaddr_in *) malloc ( sizeof ( struct sockaddr_in ) );
	
	server->sin_family			= AF_INET;
	server->sin_addr.s_addr		= INADDR_ANY;
	server->sin_port 			= htons ( port );
	
	//effettuo la Bind e gestisco l'eventuale errore
	//questo ciclo tenta la bind nel caso in cui non la porta non sia libera
	int bindErr;

	//rende socket non bloccante
	//fcntl( sockfd, F_SETFL, O_NONBLOCK );

	do {
		bindErr = bind( sockfd, ( struct sockaddr *)server , sizeof( struct sockaddr_in ) );
		int errsv = errno;
		if ( bindErr < 0 ) {
			if ( /* errsv == EINVAL || */ errsv == EADDRINUSE )
				perror ( "Bind > Error3: Nuovo tentativo di assegnazione tra 5 secondi" ), sleep ( 5 );
			else
				perror ( "Bind > Error4: exiting...." ), exit ( -1 );
		} else if ( bindErr == 0 )
			write ( STDOUT_FILENO, "Bind eseguita.\n", strlen ( "Bind eseguita.\n" ) );  
	} while ( bindErr < 0 );

	//avvio la Listen su un numero N di operatori
	int listErr = listen( sockfd , maxOperatorsQueue );
	if ( listErr == -1 )
		perror ( "Errore di listen" ), exit ( -1 );

	char portH[6];
	sprintf ( portH, "%d\n", port );
	char *message = "In attesa di connessioni...su porta: ";
	write ( STDOUT_FILENO, message, strlen ( message ) );
	write ( STDOUT_FILENO, portH, strlen ( portH ) );

	return sockfd;
}

void connectionManager ( int sockfd, int opNumber, int kPackages, Package *handler,
                         struct sockaddr_in client, int clientsize ) {
	
	char 			tids[32];
	int 			client_sock;	
	int 			tid;

	pthread_t 		thread_id;
	pthread_t 		*thread_arr = ( pthread_t *) malloc ( opNumber * sizeof ( pthread_t ) );	
	tid 			= 0;
	maxThread 		= 0;
	clientsize 		= sizeof( struct sockaddr_in );
	char *mess00	= "Connessione accettata\n";
	char *mess01	= "Thread assegnato\n";

	while ( 1 ) {

		if ( maxThread < opNumber ) {
			client_sock = accept( sockfd, ( struct sockaddr *)&client,
			                     ( socklen_t*)&clientsize );
			if ( client_sock != -1 ) {
				write ( STDOUT_FILENO, mess00, strlen ( mess00 ) );
				Passaggio param_pass;
				memset (&param_pass, 0, sizeof ( Passaggio ) );
				param_pass.sockfd		= client_sock;
				param_pass.kPackages 	= kPackages;
				param_pass.handler 		= handler;
				
				if ( (pthread_create (&thread_arr[maxThread] , NULL , thread_connection_handler , ( void *) &param_pass )) == 0 ) {
					sprintf  ( tids, "nuovo tid: [%d] ", tid );
					tid++;

					if ( ( pthread_mutex_lock (&maxThreadsMutex ) ) == 0 ) {
						maxThread++;
						pthread_mutex_unlock (&maxThreadsMutex );
					} else
						perror ( "server-mutex: errore" );

				} else 
					perror ( "impossibile creare il thread" );
			//
			} else {
				perror ( "impossibile accettare la connessione" );
			}
		}
	}
}

/*THREAD CONNESSIONE*/
void *thread_connection_handler ( void *parametri ) {
	signal( SIGPIPE ,SIG_IGN );
	Passaggio *			tmp 			= ( Passaggio *) parametri;
	Package *			handler 		= tmp->handler;
	int 				client_sock 	= tmp->sockfd;
	int 				kPackages 		= tmp->kPackages;

	threadClientInit ( client_sock, handler, kPackages );
	talkWithClient( client_sock, handler );
	perror ( "socket client: " );
	write ( STDOUT_FILENO, "\nfine comunicazioni\n", sizeof ( "\nfine comunicazioni\n" ) );	
	/*
	all'uscita del thread, lo elimino dalla "coda" dei thread attivi e libero
	uno slot per la connessione. impiego il mutex per poter scrivere sulla
	variabile globale
	*/
	pthread_mutex_lock (&maxThreadsMutex );
	maxThread--;
	pthread_mutex_unlock (&maxThreadsMutex );

	return(( void *)0 ); 
}

void threadClientInit ( int sockfd, Package *handler, int kPackages ) {
	write ( STDOUT_FILENO, "Init iniziata\n", 14 );
	int i = 0;
	char *message;
	Package *current = handler;
	//ATTENZIONE CONTROLLA CHE QUESTA CLAUSOLA FUNZIONI
	//IMPLEMENTARE COMANDO SMISTA
	if ( current == NULL ) {
		while ( current == NULL );
	}
	//pthread_mutex_lock (&packageMutex );
	while ( current != NULL && i < kPackages ) {
		if ( current->stato_articolo == STORAGE ) {
			pthread_mutex_lock (&current->m_lock );
			//leggi pacchetto
			current->stato_articolo = TOBEDELIVERED;
			//pkg_print ( current );
			//codifica e invia pacchetto
			message = encodePkgForTransmission ( current );
			write ( sockfd, message, strlen ( message ) );
			Package *prev = current;
			current = current->next;
			pthread_mutex_unlock (&prev->m_lock );
			i++;
			free ( message );
			//usleep ( 500000 );
			//usleep ( 50000 );
		} else {
			pthread_mutex_lock (&current->m_lock );
			Package *prev = current;
			current = current->next;
			pthread_mutex_unlock (&prev->m_lock );
		}
		//pthread_mutex_unlock (&prev->m_lock );
	}
	//pthread_mutex_unlock (&packageMutex );
	write ( sockfd, "EOM#\n", sizeof ( "EOM#\n" ) );
	write ( STDOUT_FILENO, "Init Terminata\n", 15 );
}

void talkWithClient ( int client_sock, Package *handler ) {
	int command = 0;
	while ( command != -1 ) {
		//write ( STDOUT_FILENO, "in attesa di un messaggio:\n", 27 );
		char *strbuffer;
		command = getCommand ( client_sock, &strbuffer );
		//output per debug
		if ( strbuffer != NULL ) {
			write ( STDOUT_FILENO, "comando: ", sizeof ( "comando: " ) );		
			write ( STDOUT_FILENO, strbuffer, strlen ( strbuffer ) );
			write ( STDOUT_FILENO, "\n", 1 );	
			command  = commandSwitchServer ( command, strbuffer, handler, client_sock );
			//write ( STDOUT_FILENO, "\nmessaggio terminato\n", 20 );

		}
	}

}

int commandSwitchServer ( int command, char *cmdPointer,
                          Package *handler, int sockfd ) {
	char *err01 = "warning! comando non valido!\n";
	char mess00[] = "elencaserver\n";
	char mess01[] = "pacchetto consegnato\n";
	char mess02[] = "pacchetto ritirato\n";
	char mess03[] = "pacchetto smistato\n";
	
	switch ( command ) {
		case -1:
		command = -1;
		break;
		case ELENCASERVER:
		/*ELENCA STAMPA LA LISTA REMOTA*/
			write ( STDOUT_FILENO, mess00, strlen ( mess00 ) );
			elencaserver_server ( sockfd, handler );
		break;
		case CONSEGNATO:
			write ( STDOUT_FILENO, mess01, strlen ( mess01 ) );
			consegnato_server ( sockfd, cmdPointer, handler );			
		break;
		case RITIRATO:
			write ( STDOUT_FILENO, mess02, strlen ( mess02 ) );
			ritirato_server ( sockfd, cmdPointer, handler );
			//pkglist_print_r ( handler );
		break;
		case SMISTA:
			write ( STDOUT_FILENO, mess03, strlen ( mess03 ) );
			smista_server ( sockfd, cmdPointer, handler );		
		break;					
		default:
			write ( STDOUT_FILENO, err01, strlen ( err01 ) );
		break;
	}
	return command;
}
/**elenca i file locali al client*/
void elencaserver_server ( int sockfd, Package *handler ) {

	int i = 0;
	char *message;
	Package *current = handler;
	if ( current == NULL ) {
		while ( current == NULL );
	}
	//pthread_mutex_lock (&packageMutex );
	while ( current != NULL ) {
			pthread_mutex_lock (&current->m_lock );
			//leggi pacchetto
			pkg_print ( current );
			//codifica e invia pacchetto

			message = encodePkgForTransmission ( current );
			write ( sockfd, message, strlen ( message ) );
			i++;
			free ( message );
			//usleep ( 500000 );
			//usleep ( 50000 );
			Package *prev = current;
			current = current->next;
			pthread_mutex_unlock (&prev->m_lock );
	}
	//pthread_mutex_unlock (&packageMutex );
	write ( sockfd, "EOM#\n", sizeof ( "EOM#\n" ) );

}
/**cambia lo stato di un articolo a CONSEGNATO (DELIVERED) */
void consegnato_server ( int sockfd, char *strbuffer, Package *handler ) {
	int 		tokensNumber	 = 2;
	int 		lenght 			= strlen ( strbuffer );
	char *		str[tokensNumber];
	strbuffer[lenght-1] = '\0';
	memsetString ( str, tokensNumber);
	getTokens ( str, strbuffer, tokensNumber );
	Package * result = pkg_find_mutex ( handler, str[1] );

	if ( result != NULL ) {
		pthread_mutex_lock (&result->m_lock );
		result->stato_articolo = DELIVERED;
		pthread_mutex_unlock (&result->m_lock );		
	} else {
		perror ( "attenzione: pacchetto non valido o non esistente\n" );
	}

	memset ( strbuffer, 0,  STRING * sizeof ( char ) );
	freeArray ( str, tokensNumber );
	free ( strbuffer );

}
/**aggiunge in coda un articolo e gli assegna lo stato RITIRATO (COLLECTED)*/
void ritirato_server ( int sockfd, char *strbuffer, Package *handler ) {

	char 		messNo[] 		= "inserimento non concesso: codice pacchetto gia' esistente\n";
	char 		messOk[] 		= "inserimento avvenuto con successo\n";
	char 		no[] 			= "NOTOK\n";
	char 		ok[] 			= "INSOK\n";
	int 		lenght 			= strlen ( strbuffer );
	int 		tokensNumber	= 3;
	char * 		str[tokensNumber];

	strbuffer[lenght-1] 		= '\0';
	memsetString ( str, tokensNumber);

	getTokens ( str, &strbuffer[9], tokensNumber );
	Package * result = pkg_find_mutex ( handler, str[0] );
	if ( result == NULL ) {
		write ( sockfd, ok, strlen ( ok ) );
		write ( STDOUT_FILENO, messOk, strlen ( messOk ) );
		
		Status status = COLLECTED;
		handler = pkg_enqueue_r_mutex ( handler, str, status );

	} else {

		write ( sockfd, no, strlen ( no ) );
		write ( STDOUT_FILENO, messNo, strlen ( messNo ) );

	}

	memset ( strbuffer, 0, STRING * sizeof ( char ) );
	freeArray ( str, tokensNumber );
	free ( strbuffer );

/**/
}
/**cambia lo stato di un articolo da RITIRATO a IN MAGAZZINO (STORED)*/
void smista_server ( int sockfd, char *strbuffer, Package *handler ) {
	int 		tokensNumber 	= 2;
	int 		lenght 			= strlen ( strbuffer );
	char * 		str[tokensNumber];

	strbuffer[lenght-1] = '\0';
	memsetString ( str, tokensNumber);
	getTokens ( str, strbuffer, tokensNumber );
	Package * result = pkg_find_mutex ( handler, str[1] );

	if ( result != NULL ) {
		pthread_mutex_lock (&result->m_lock );
		result->stato_articolo = STORAGE;
		pthread_mutex_unlock (&result->m_lock );		
	} else {
		perror ( "attenzione: pacchetto non valido o non esistente\n" );
	}
	
	memset ( strbuffer, 0,  STRING * sizeof ( char ) );
	freeArray ( str, tokensNumber );
	free ( strbuffer );

}