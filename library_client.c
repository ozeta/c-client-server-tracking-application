/*=============================================================================
  Nome: server.c
  Autori:
	MARCO CARROZZO     	N86/1240
	MAURIZIO DEL PRETE 	N86/783

	Progetto: Corriere Espresso
  ===========================================================================*/

#include "library_client.h"

Package *initClient ( int sockfd, Package *handler ) {
	//pkg handler, socket, tokens, status
	//initClient si mette in attesa di ricevere pacchetti, anche
	//se il server ne è momentaneamene sprovvisto
	while (( handler = createList ( handler, sockfd,  4, -1, 1 ) ) == NULL ) {
		usleep ( 50000 );
	}

	//pkglist_print_r ( handler );

	return handler;
}


int initClientSocket ( char **argv ) {

	int sockfd;
	char *			mess00 	= "ip agganciato...\n";
	char *			mess01 	= "socket creato...\n";
	char *			mess02 	= "connessione eseguita.\n\n";
	char *			mess03 	= "impossibile connettersi.\n";
	
	struct addrinfo hints, *res;
	memset ( &hints, 0, sizeof ( hints ) );
	
	hints.ai_family			= AF_INET; 															//protocollo ipv4
	hints.ai_socktype   	= SOCK_STREAM; 														//tcp
	hints.ai_flags			= AI_PASSIVE; 														//INADDR_ANY
		
	if ( getaddrinfo ( argv[1], argv[2], &hints, &res ) == -1 )									//argv[1] = ip; argv[2] = port
		perror ( "Error1: " ), exit ( -1 );
	write ( STDOUT_FILENO, mess00, strlen ( mess00 ) );
	sockfd = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
	if ( sockfd == -1 )
		perror ( "Error2: " ), exit ( -1 );
	write ( STDOUT_FILENO, mess01, strlen ( mess01 ) );

	int timeout = 12;
	while (( connect ( sockfd, res->ai_addr, res->ai_addrlen ) < 0 ) && timeout > 0 ) {			//se la porta e' occupata, tento di riappropriarmene per timeout volte
		perror ( "connect >  Nuovo tentativo di connessione tra 5 secondi" ), sleep ( 5 );
		timeout--;
	}
	
	if ( timeout == 0 )
		write ( STDERR_FILENO, mess03, strlen ( mess03 ) ), exit ( -1 );						//se il timeout scade il programma chiude
	
	write ( STDOUT_FILENO, mess02, strlen ( mess02 ) );
	return sockfd;
}


Package *commandSwitch ( int command, char *strbuffer, Package *handler, int sockfd ) {
	char *err01 = "warning! comando non valido!\n";

	switch ( command ) {
		case ELENCASERVER:
		/*STAMPA LA LISTA REMOTA*/
			//write ( STDOUT_FILENO, strbuffer, strlen ( strbuffer ) );
			elencaserver_client ( sockfd, strbuffer );
		break;
		case CONSEGNATO:
			//write ( STDOUT_FILENO, "switch-> consegnato\n", strlen ( "switch-> consegnato\n" ) );
			handler = consegnato_client ( sockfd, strbuffer, handler );
		break;
		case RITIRATO:
			//write ( STDOUT_FILENO, "switch-> ritirato\n", strlen ( "switch-> ritirato\n" ) );
			ritirato_client ( sockfd, strbuffer, handler );
		break;
		case SMISTA:
			//write ( STDOUT_FILENO, "switch-> smista\n", strlen ( "switch-> smista\n" ) );
			handler = smista_client ( sockfd, strbuffer, handler );
		break;
		case ELENCA:
			//write ( STDOUT_FILENO, "switch-> elenca:\n", strlen ( "switch-> elenca:\n" ) );
			pkglist_print_r ( handler );
		break;						
		case ESCI:
			/*
			//list_dump (handler, outFD, 0);
			//list_delete_r ( handler );
			handler = NULL;
			sendMessage ( sockfd, "esci\n");
			*/
		break;
		default:
			write ( STDOUT_FILENO, err01, strlen ( err01 ) );
		break;

	}
	return handler;
}


void elencaserver_client ( int sockfd, char *commandLine ) {

	int 		check = 1;
	char *		ptr;

	char *strbuffer = stringMalloc ();	
	sendMessage ( sockfd, commandLine );
	//write ( sockfd, "\n", 1 );
	
	while ( check != 0 && ( readLine ( sockfd, strbuffer ) ) > 0 ) {
		//funzione di libreria che cerca una sottostringa
		//in una sottostringa e restituisce il puntatore
		ptr = strstr ( strbuffer, "EOM#" );
		if ( ptr == NULL ) {
			strbuffer = decodePkgfromTransmission ( strbuffer );
			write ( STDOUT_FILENO, strbuffer, strlen ( strbuffer ) );
			//write ( STDOUT_FILENO, "\n", 1 );
		} else
			check = 0;
		memset ( strbuffer, 0,  STRING * sizeof ( char ) );	
	}

	free ( strbuffer );
}


int checkCommandInput ( char *strbuffer, int parameters ) {

	int i = 0;
	int end = strlen ( strbuffer );
	int res = 0;
	while ( i < end ) {
		if ( strbuffer[i] == '#')
			res++;	
		i++;
	}

	if ( parameters == res )
		return 1;
	else
		return 0;
}

Package *consegnato_client ( int sockfd, char *strbuffer, Package *handler ) {

		//consegnato#codice
		//cercapacchetto
		//inviapacchetto
		//cancellapacchetto
	int check;
	if ( ( check = checkCommandInput ( strbuffer, 1 ) ) != 0 ) {

		int 		tokensNumber 		= 2;
		int 		lenght 				= strlen ( strbuffer );
		char *		str[tokensNumber];

		strbuffer[lenght-1] 			= '\0';
		stringArrayMalloc ( str, tokensNumber);
		getTokens ( str, strbuffer, tokensNumber );

		Package *result = pkg_find_r ( handler,str[1] );
		if ( result != NULL && result->stato_articolo == TOBEDELIVERED ) {
			strbuffer[lenght-1] = '\n';		
			if ( ( sendMessage ( sockfd, strbuffer ) ) != -1 ) {
				handler = pkg_delete_r ( handler, str[1] );							
			} else {
				perror ( "impossibile inviare il messaggio. operazione abortita" );
			}
		} else {
			char warn0[] = "attenzione: pacchetto non valido o non esistente\n";
			write ( STDOUT_FILENO, warn0, strlen ( warn0 ) );
		}
		freeArray ( str, tokensNumber );
	} else {
		char warn1[] = "attenzione: comando non corretto\n";
		char warn2[] = "esempio: consegnato#codice\n";
		write ( STDOUT_FILENO, warn1, strlen ( warn1 ) );
		write ( STDOUT_FILENO, warn2, strlen ( warn2 ) );
	}

	return handler;
}

void ritirato_client ( int sockfd, char *strbuffer, Package *handler ) {
	//array semidinamico: riceve l'input dalla funzione
	//chiamante
	int 	check;
	char 	result[32];
	char 	messOk[] 		= "inserimento avvenuto con successo\n";
	char 	messNo[] 		= "impossibile aggiungere pacchetto: codice pacchetto gia' esistente su lista remota\n";

	memset ( result, 0, 32 * sizeof( char ) );

	if ( ( check = checkCommandInput ( strbuffer, 3 ) ) != 0 ) {

		char 		no[] 				= "NOTOK";
		char 		ok[] 				= "INSOK";
		int 		tokensNumber 		= 3;
		char *		str[tokensNumber];
		
		if ( (sendMessage ( sockfd, strbuffer ) ) > 0 )
			check = readLine ( sockfd, result );
		//write ( STDOUT_FILENO, strbuffer, strlen ( strbuffer ) );				

		if (( strcmp ( result, ok ) ) == 0 && check != -1 ) {
			Status status 		= COLLECTED;
			int lenght 			= strlen ( strbuffer );

			strbuffer[lenght-1] = '\0';

			stringArrayMalloc ( str, tokensNumber);
			getTokens ( str, &strbuffer[9], tokensNumber );
			handler = pkg_enqueue_r ( handler, str, status );

			write ( STDOUT_FILENO, messOk, strlen ( messOk ) );
			freeArray ( str, tokensNumber );
		} else {
			write ( STDOUT_FILENO, messNo, strlen ( messNo ) );
		}
			
	} else {
		char warn1[] = "attenzione, comando non corretto\n";
		char warn2[] = "esempio: ritirato#codice#descrizione#indirizzo\n";
		write ( STDOUT_FILENO, warn1, strlen ( warn1 ) );
		write ( STDOUT_FILENO, warn2, strlen ( warn2 ) );
	}

}


Package *smista_client ( int sockfd, char *strbuffer, Package *handler ) {
	int check;
	if ( ( check = checkCommandInput ( strbuffer, 1 ) ) != 0 ) {

		int 		tokensNumber 		= 2;
		int 		lenght 				= strlen ( strbuffer );
		char *		str[tokensNumber];

		strbuffer[lenght-1] = '\0';
		stringArrayMalloc ( str, tokensNumber);
		getTokens ( str, strbuffer, tokensNumber );

		Package *result = pkg_find_r ( handler,str[1] );

		if ( result != NULL && result->stato_articolo == COLLECTED ) {
			strbuffer[lenght-1] = '\n';		
			if (( sendMessage ( sockfd, strbuffer ) ) != -1 ) {
				handler = pkg_delete_r ( handler, str[1] );							
			} else {
				perror ( "impossibile inviare il messaggio. operazione abortita" );
			}
		} else {
			char warn0[] = "attenzione: pacchetto non valido o non esistente\n";
			write ( STDOUT_FILENO, warn0, strlen ( warn0 ) );
		}
		freeArray ( str, tokensNumber );
	} else {
		char warn1[] = "attenzione: comando non corretto\n";
		char warn2[] = "esempio: smista#codice\n";
		write ( STDOUT_FILENO, warn1, strlen ( warn1 ) );
		write ( STDOUT_FILENO, warn2, strlen ( warn2 ) );
	}

	return handler;
}
