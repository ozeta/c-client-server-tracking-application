﻿/*=============================================================================
  Nome: server.c
  Autori:
	MARCO CARROZZO     	N86/1240
	MAURIZIO DEL PRETE 	N86/783

	Progetto: Corriere Espresso
  ===========================================================================*/

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
	
	int 			client_sock;	
	int 			tid;

	pthread_t 		thread_id;
	pthread_t 		*thread_arr = ( pthread_t *) malloc ( opNumber * sizeof ( pthread_t ) );	

	maxThread 		= 0;
	clientsize 		= sizeof( struct sockaddr_in );
	
	char *mess00	= "Connessione accettata\n";
	char *mess01	= "Thread assegnato\n";

	while ( 1 ) {
		//leggo dalla variabile globale il numero di connessioni attivate
		pthread_mutex_lock ( &maxThreadsMutex );
		int threadCheck = maxThread;
		pthread_mutex_unlock ( &maxThreadsMutex );
		//se ok, avvio la procedura per creare il nuovo thread
		if ( threadCheck < opNumber ) {
		//accetto la connessione in entrata
			client_sock = accept( sockfd, ( struct sockaddr *)&client,
			                     ( socklen_t* )&clientsize );
			if ( client_sock != -1 ) {
				write ( STDOUT_FILENO, mess00, strlen ( mess00 ) );
				//imposto una struttura temporanea per passare i parametri al thread
				Passaggio param_pass;
				memset ( &param_pass, 0, sizeof ( Passaggio ) );
				param_pass.sockfd		= client_sock;
				param_pass.kPackages 	= kPackages;
				param_pass.handler 		= handler;
				//creo il thread e passo la struttura di parametri
				if ( (pthread_create ( &thread_arr[maxThread] , NULL , thread_connection_handler , ( void *) &param_pass )) == 0 ) {
					//blocco il contatore di connessioni e lo aggiorno
					if ( ( pthread_mutex_lock ( &maxThreadsMutex ) ) == 0 ) {
						maxThread++;
						pthread_mutex_unlock ( &maxThreadsMutex );
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
	free (thread_arr);
}

/*THREAD CONNESSIONE*/
void *thread_connection_handler ( void *parametri ) {
	signal( SIGPIPE ,SIG_IGN );
	Passaggio *			tmp 			= ( Passaggio * ) parametri;
	Package *			handler 		= tmp->handler;
	int 				client_sock 	= tmp->sockfd;
	int 				kPackages 		= tmp->kPackages;
	int 				check 			= 0;
	char *				mess1 			= "nuova Init Terminata\n";
	char *				mess2 			= "\nfine comunicazioni\n";
	while ( ( check = threadClientInit ( client_sock, handler, kPackages ) ) == 0)
		sleep ( 5 );
	if ( check != -1 ) {
		write ( STDOUT_FILENO, mess1, strlen ( mess1 ) );
		talkWithClient( client_sock, handler );
		write ( STDOUT_FILENO, mess2, strlen ( mess2 ) );
	}
	/*
	all'uscita del thread, lo elimino dalla "coda" dei thread attivi e libero
	uno slot per la connessione. impiego il mutex per poter scrivere sulla
	variabile globale
	*/
	pthread_mutex_lock ( &maxThreadsMutex );
	maxThread--;
	pthread_mutex_unlock ( &maxThreadsMutex );
	close ( client_sock );
	return(( void *)0 ); 
}

int threadClientInit ( int sockfd, Package *handler, int kPackages ) {

	char *			message;
	int 			check 			= 1;
	int 			i 				= 0;
	Package *		current 		= handler;

	
	while ( check >= 0 && current != NULL && i < kPackages ) {
		if ( current->stato_articolo == STORAGE ) {
			pthread_mutex_lock ( &current->m_lock );
			//leggi pacchetto
			current->stato_articolo = TOBEDELIVERED;
			//pkg_print ( current );
			//codifica e invia pacchetto
			message = encodePkgForTransmission ( current );
			check 	= sendMessage ( sockfd, message );
			Package *prev = current;
			current = current->next;
			pthread_mutex_unlock ( &prev->m_lock );
			free ( message );
			i++;
			//usleep ( 500000 );
			//usleep ( 50000 );
		} else {
			pthread_mutex_lock ( &current->m_lock );
			Package *prev = current;
			current = current->next;
			pthread_mutex_unlock ( &prev->m_lock );
		}

	}

	if (check >= 0)
		sendMessage ( sockfd, "EOM#\n" );
	else
		i = -1;
	
	return i;
}

void talkWithClient ( int client_sock, Package *handler ) {
	int command = 0;
	while ( command != -1 ) {
		char *strbuffer;
		command = getCommand ( client_sock, &strbuffer );
		//output per debug
		if ( strbuffer != NULL ) {
			write ( STDOUT_FILENO, "comando: ", sizeof ( "comando: " ) );		
			write ( STDOUT_FILENO, strbuffer, strlen ( strbuffer ) );
			write ( STDOUT_FILENO, "\n", 1 );	
			command  = commandSwitchServer ( command, strbuffer, handler, client_sock );

		}
	}

}

int commandSwitchServer ( int command, char *cmdPointer,
                          Package *handler, int sockfd ) {
	char 	err01[]		= "warning! comando non valido!\n";
	char 	mess00[] 	= "elencaserver\n";
	char 	mess01[] 	= "pacchetto consegnato\n";
	char 	mess02[] 	= "pacchetto ritirato\n";
	char 	mess03[] 	= "pacchetto smistato\n";
	char 	mess04[]	= "client disconnesso\n";
	switch ( command ) {
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
		case ESCI:
			write ( STDOUT_FILENO, mess04, strlen ( mess04 ) );
			command = -1;		
		break;
		default:
			write ( STDOUT_FILENO, err01, strlen ( err01 ) );
		break;
	}
	return command;
}
/**elenca i file locali al client*/
void elencaserver_server ( int sockfd, Package *handler ) {


	int 		check 		= 1;
	Package *	current 	= handler;

	while ( current != NULL && check >= 0) {
		pthread_mutex_lock ( &current->m_lock );
		//leggi pacchetto
		pkg_print ( current );
		//codifica e invia pacchetto

		char *message = encodePkgForTransmission ( current );
		check = sendMessage ( sockfd, message );
		free ( message );
		//usleep ( 500000 );
		//usleep ( 50000 );
		Package *prev 	= current;
		current 		= current->next;
		pthread_mutex_unlock ( &prev->m_lock );
	}
	if (check >= 0)
		sendMessage ( sockfd, "EOM#\n" );
}
/**cambia lo stato di un articolo a CONSEGNATO (DELIVERED) */
void consegnato_server ( int sockfd, char *strbuffer, Package *handler ) {
	int 		tokensNumber	= 2;
	int 		lenght 			= strlen ( strbuffer );
	char *		str[tokensNumber];
	strbuffer[lenght-1] = '\0';

	stringArrayMalloc ( str, tokensNumber);
	getTokens ( str, strbuffer, tokensNumber );
	Package * result = pkg_find_mutex ( handler, str[1] );

	if ( result != NULL ) {
		pthread_mutex_lock ( &result->m_lock );
		result->stato_articolo = DELIVERED;
		pthread_mutex_unlock ( &result->m_lock );		
	} else {
		perror ( "attenzione: pacchetto non valido o non esistente\n" );
	}

	memset ( strbuffer, 0,  STRING * sizeof ( char ) );
	freeArray ( str, tokensNumber );
	free ( strbuffer );

}
/**aggiunge in coda un articolo e gli assegna lo stato RITIRATO (COLLECTED)*/
void ritirato_server ( int sockfd, char *strbuffer, Package *handler ) {

	char 		messNo[] 			= "inserimento non concesso: codice pacchetto gia' esistente\n";
	char 		messOk[] 			= "inserimento avvenuto con successo\n";
	char 		no[] 				= "NOTOK\n";
	char 		ok[] 				= "INSOK\n";
	int 		lenght 				= strlen ( strbuffer );
	int 		tokensNumber		= 3;
	int 		check 				= 1;
	char * 		str[tokensNumber];

	strbuffer[lenght-1] 			= '\0';
	stringArrayMalloc ( str, tokensNumber);

	getTokens ( str, &strbuffer[9], tokensNumber );
	Package * result = pkg_find_mutex ( handler, str[0] );
	if ( result == NULL ) {
		check = sendMessage ( sockfd, ok );
		if (check >= 0 ) {
			Status status = COLLECTED;
			handler = pkg_enqueue_r_mutex ( handler, str, status );
			write ( STDOUT_FILENO, messOk, strlen ( messOk ) );
		} else
			write ( STDOUT_FILENO, messNo, strlen ( messNo ) );
	} else {

		sendMessage ( sockfd, no );
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

	strbuffer[lenght-1] 		= '\0';
	stringArrayMalloc ( str, tokensNumber);
	getTokens ( str, strbuffer, tokensNumber );
	Package * result = pkg_find_mutex ( handler, str[1] );

	if ( result != NULL ) {
		pthread_mutex_lock ( &result->m_lock );
		result->stato_articolo = STORAGE;
		pthread_mutex_unlock ( &result->m_lock );		
	} else {
		perror ( "attenzione: pacchetto non valido o non esistente\n" );
	}
	
	memset ( strbuffer, 0,  STRING * sizeof ( char ) );
	freeArray ( str, tokensNumber );
	free ( strbuffer );

}

