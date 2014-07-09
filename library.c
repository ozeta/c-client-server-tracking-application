#include "library.h"
//todo: devo inizializzare il semaforo.
/*===========================================================================*/
/*
	Nome: server.c
	Autori: Marco Carrozzo, Maurizio Del Prete
	Progetto: Corriere Espresso



	libreria di funzioni generiche e condivise



/*===========================================================================*/


/*===========================================================================*/
/*
1 ) operazioni su lista puntata
pkg_print stampa la lista intera
pkg_malloc alloca memoria per la struttura package
pkg_enqueue_r inserisce una struttura package in coda
pkg_push_r inserisce una struttura package in testa
pkg_delete_r cancella una struttura package dalla lista
list_delete_r cancella la lista
pkg_find_r cerca una struttura package nella lista
*/
/*===========================================================================*/


/**questo main serve solo per compilare il file in standalone. va rimosso
	prima della consegna*//*
int main () {
	return 0;
}
*/
/*
per motivi di efficienza ( velocita'/memoria ) di stack e heap preferisco
 chiamare prima la visita ricorsiva sul nodo successivo e poi
 effettuare le operazioni sul nodo.
*/

char *cliCommands[5] = {
	"ELENCASERVER",
	"CONSEGNATO",
	"RITIRATO",
	"SMISTA",
	"ELENCA"
};

void pkglist_print_r ( Package *handler ) {
	if ( handler != NULL ) {
		pkglist_print_r( handler->next );	
		pkg_print ( handler );

	}
}

void pkg_print ( Package *handler ) {
	if ( handler != NULL ) {
		char *message = ( char *) malloc ( 256 * sizeof ( char ) );
		char status[20];
		memset ( message, '\0', strlen ( message ) );

		switch ( handler->stato_articolo ) {
			case STORAGE: strcpy ( status, "STORAGE" ); break;
			case TOBEDELIVERED: strcpy ( status, "TOBEDELIVERED" ); break;
			case DELIVERED: strcpy ( status, "DELIVERED" ); break;
			case COLLECTED: strcpy ( status, "COLLECTED" ); break;
			default: strcpy ( status, "boh?" ); break;
		}
		sprintf ( message, "%s , %s , %s . STATO: %s\n", handler->codice_articolo,
//		sprintf ( message, "Item= %s , %s , %s . STATO: %s\n", handler->codice_articolo,
				handler->descrizione_articolo,
				handler->indirizzo_destinazione,
				status );	
		write ( STDOUT_FILENO, message, strlen ( message ) );
		free ( message );
	}
}


/**
pkg_initialize
status: -1 		-> leggi lo stato da buffer
status: [0,3]	-> imposta buffer manualmente
*/
Package * pkg_initialize ( char **buffer, int status ) {
	int			err;	
	int			i 			= 0;
	char 		*err00 		= "err00: memoria esaurita\n";
	char 		*err01 		= "err01: impossibile inizializzare il mutex\n";	
	Package 	*newPkg   	= ( Package *) malloc ( sizeof ( Package ) );
	//controllo allocazione memoria
	if ( newPkg == NULL ) {
		write ( STDERR_FILENO, err00, strlen ( err00 ) );
		return NULL;
	}
	memset ( newPkg, 0, sizeof ( Package ) );

	strcpy ( newPkg->codice_articolo, buffer[0] );
	strcpy ( newPkg->descrizione_articolo, buffer[1] );
	strcpy ( newPkg->indirizzo_destinazione, buffer[2] );
	if ( status != -1 )
		newPkg->stato_articolo = status;
	else {
		i = atoi ( buffer[3] );
		newPkg->stato_articolo = i;	
	}
	newPkg->next = NULL;
	if (( err = pthread_mutex_init (&newPkg->m_lock, NULL ) ) != 0 ) 
		perror ( "Impossibile allocare il mutex per i thread" ), exit ( -1 );
	return newPkg;
}

Package * pkg_enqueue_r ( Package * handler, char **buffer, int status ) {
	if ( handler == NULL ){
		Package * nuovo_nodo = pkg_initialize ( buffer, status );
		if ( nuovo_nodo != NULL ) {
			nuovo_nodo->next = handler;
			handler	= nuovo_nodo;
		}
	} else
		handler->next = pkg_enqueue_r ( handler->next, buffer, status );

	return handler;
}

Package * pkg_push_r ( Package * handler, char **buffer, int status ) {
	Package * newPkg = pkg_initialize ( buffer, status );
	if ( handler != NULL )
		newPkg->next = handler;
	return newPkg;
}

Package * pkg_delete_r ( Package * handler, char *buffer0 ) {
	if ( handler == NULL ){

	} else if ( strcmp ( buffer0, handler->codice_articolo ) == 0 ){
		Package * nodo = handler->next;
		free( handler );
		handler = nodo;
	} else {
		handler->next = pkg_delete_r ( handler->next, buffer0 );
	}

	return handler;
}

void list_delete_r ( Package * handler ){

	if ( handler != NULL ){
		list_delete_r ( handler->next );
		handler->next = NULL;
		free ( handler );

	}
}



/**funzione di ricerca base basata su codice articolo*/
Package * pkg_find_r ( Package * handler, char *pkgCode ) {

	Package * result = NULL;

	//rettifica <= pkgCode
	if ( handler != NULL ) {
		if ( strcmp ( pkgCode, handler->codice_articolo ) == 0 ) {
			result = handler;

		} else {
			result = pkg_find_r ( handler->next, pkgCode );
		}

	}

	return result;
}

Package * pkg_find_mutex ( Package * handler, char *pkgCode ) {

	Package * result = NULL;
	Package * current = handler;
	while ( current != NULL && result == NULL ) {
		pthread_mutex_lock (&current->m_lock );

		if (( strcmp ( current->codice_articolo, pkgCode ) ) == 0 )
			result = current;

		Package *prev = current;
		current = current->next;
		pthread_mutex_unlock (&prev->m_lock );		
	}
	return result;
}

Package * pkg_enqueue_r_mutex ( Package * handler, char **buffer, int status ) {
	pthread_mutex_lock (&handler->m_lock );
	if ( handler == NULL ){
		Package * nuovo_nodo = pkg_initialize ( buffer, status );
		if ( nuovo_nodo != NULL ) {

			nuovo_nodo->next = handler;
			handler	= nuovo_nodo;
		}
	} else
		handler->next = pkg_enqueue_r ( handler->next, buffer, status );
	pthread_mutex_unlock (&handler->m_lock );
	return handler;
}

/**funzione di ricerca basata sullo stato di magazzino dell'articolo*/
Package * getStoredPackage_r ( Package * handler, int status ) {

	Package * result = NULL;

	if ( handler != NULL ) {
		if ( handler->stato_articolo == status ) {
			result = handler;

		} else {
			result = getStoredPackage_r ( handler->next, status );
		}
	}

	return result;
}

/**
funzione che prende in input l'handler della lista, il file di testo, il
 numero di token da analizzare.
 la funzione alloca lo spazio per un numero sufficiente di stringhe temporanee
 dopodiché inizia a leggere linea per linea il file di testo, salvando in strbuffer
 il contenuto della linea. usa la procedura
 getTokens per salvare nell'array di stringhe allocato il contenuto della stringa.
 una volta ottenuti i token, li salva nella lista.
 successivamente viene azzerato il contenuto nelle stringhe di passaggio, e si
 passa alla linea successiva.
 al termine, vengono liberate le stringhe temporanee.
*/

Package * createList ( Package *handler, int inputFD, int tokensNumber, int status, int print ) {
	//array semidinamico: riceve l'input dalla funzione
	//chiamante
	char 		*str[tokensNumber];
	char 		*ptr;
	int 		i;
	int 		check = 1;
	
	for ( i = 0; i < tokensNumber; i++) {
		str[i] = ( char *) malloc ( 256 * sizeof ( char ) );
		if ( str[i] == NULL )
			perror ( "memoria esaurita" ), exit ( -1 );
		memset ( str[i], '\0', strlen ( str[i] ) );
	}
	char *strbuffer = malloc ( 256 * sizeof ( char ) );
	if ( strbuffer == NULL )
		perror ( "memoria esaurita" ), exit ( -1 );	
	memset ( strbuffer, 0, strlen ( strbuffer ) );

	while ( check != 0 && ( readLine ( inputFD, strbuffer ) ) > 0 ) { //prendo in ingresso una riga
		//funzione di libreria che cerca una sottostringa
		//in una sottostringa e restituisce il puntatore
		ptr = strstr ( strbuffer, "EOM#" ); //controllo se è presente il codice di fine messaggio
		if ( ptr == NULL ) {
			getTokens ( str, strbuffer, tokensNumber ); //estraggo i token dalla stringa
			/**implementazione con push su "pila"*/
			handler = pkg_push_r ( handler, str, status ); //aggiungo un pacchetto in testa
			if ( print == 1 )
				pkg_print ( handler );
			for ( i = 0; i < tokensNumber; i++) //azzero le stringhe
				memset ( str[i], '\0', strlen ( str[i] ) );
			memset ( strbuffer, 0, strlen ( strbuffer ) );
		} else
			check = 0;	
	}

	for ( i = 0; i < tokensNumber; i++) { //libero i puntatori
			free ( str[i] );
	}
	free ( strbuffer );
	
	return handler;
}
/**
prende in ingresso una linea del file di testo e la salva in una stringa
puntata.
la funzione restituisce il numero di lettere lette nella linea per controllare
che il file non sia terminato.
*/

int readLine ( int inputFD, char *strbuffer ) {
	memset ( strbuffer, 0, strlen ( strbuffer ) );
	int	i = 0;
	char c;
	while (( read ( inputFD, &c, 1 ) ) > 0 && ( c != '\n') ) {
		if ( c != '\0') //controllo che nel buffer non ci siano terminatori
			strbuffer[i++] = c;
	}
	strbuffer[i] = '\0';	

	return i;
}

/**
la procedura riempie un array di sottostringhe a partire da una
singola stringa
per ognuno dei token da leggere, la funzione chiama getTocken.
*/
void getTokens ( char *string[], char *strbuffer, int tokensNumber ) {
	int i = 0;
	int numString = 0;
	int k = 0;

	for ( i = 0; i < tokensNumber -1 ; i++) {
		strbuffer = getSubstr ( string[i], strbuffer, '#', 1 );
	}
	
	getSubstr ( string[i], strbuffer, '\0', 0 );		
}

/**
la funzione prende in ingresso il puntatore all'array in cui scrivera' la stringa,
la stringa di input, il separatore di linea e stepup.
la funzione copia carattere per carattere la stringa di ingresso nella stringa di
uscita, fintanto che non viene raggiunto il carattere separatore.
la funzione quindi restituisce il puntatore al delimitatore, cioè all'ultimo carattere
della sottostringa/stringa
a questo punto, se stepup == 1, allora verrà restituito il puntatore alla successiva
cella di memoria, in modo da poter superare il carattere di delimitazione e poter
richiamare la stessa funzione sul resto della sottostringa.
*/
char *getSubstr ( char *result, char *input, char terminal, int stepup ) {
	int i = 0;
	char *punt;
	
	while ( input[i] != terminal ) {
		result[i] = input[i++];
	}
	if ( stepup == 1 ) 
		input++;

	return input+i;
}

/*===========================================================================*/
/*
2 ) operazioni su command line arguments
*/
/*===========================================================================*/

int checkArguments ( char *argument, int argNum ) {
	int i 	= 0;
	int res = 1;

	if ( argument[i] == '-') {
		error ( 0, EINVAL, "argomento %d -> inserire un numero positivo", argNum );
		res = -1;
	} else {

		while ( res > 0 && argument[i] != '\0') {
			if ( isdigit ( argument[i] ) == 0 ) {
				error ( 0, EINVAL, "argomento %d -> inserire un numero", argNum );
				res = -1;
			}
			i++;
		}
	}

	return res;
}

/**
Il processo server riceve su linea di comando:
	intero N che definisce il numero massimo di operatori attivi;
	intero K che definisce il numero di oggetti assegnati inizialmente ad ogni operatore;
	stringa S nome del file contenente la lista degli oggetti da consegnare
*/

int serverInputCheck ( int argc, char **argv ) {
	int i = 0;
	int test;
	char argv_err0[] = "e' necessario inserire: N operatori; K oggetti; file di lettura\n";
	if ( argv[1] == NULL || argv[2] == NULL || argv[3] == NULL ) {
		write ( STDERR_FILENO, argv_err0, strlen( argv_err0 ) );
		exit( -1 );
	}
	

	//uso la funzione error per:
	//1 ) scrivere su standard error
	//2 ) richiamare un messaggio di errore standard

	/* controllo valore operatori attivi*/
	if (( test = checkArguments ( argv[1], 1 ) ) < 0 ) {
		exit( -1 );
	}
	/* controllo valore oggetti da assegnare */	
	if (( test = checkArguments ( argv[2], 2 ) ) < 0 ) {
		exit( -1 );
	}

	/* controllo file in lettura*/
	/* ENOENT = No such file or directory */
	test = open ( argv[3], O_RDONLY ); 
	if ( test < 0 )
		error ( 0, ENOENT , "file di testo non valido" ), exit ( -1 );


	if ( argv[4] != NULL && argv[4][1] == '-' && argv[4][1] == 'd')
		DEBUG = 1;

	return test;
}


int randomIntGenerator ( int inf, int sup ) {

	return rand()%( sup - inf +1 ) + inf;
}



/*===========================================================================*/
/*
3 ) operazioni su clientinput
	verifica input
*/
/*===========================================================================*/

/*
verifica che l'indirizzio ip immesso in ingresso sia valido
inet_pton converte un indirizzo ip "human readable" nella sua rappresentazione
binaria e lo salva in una struttura sockaddr_in.
la funzione restituisce:
1 successo
0 ip non valido
-1 errore ( imposta errno, recuperato da perror e stampato su standard error )
*/

int isValidIpAddress ( char *ipAddress ) {
	struct sockaddr_in test;
	int res = inet_pton ( AF_INET, ipAddress, &( test.sin_addr ) );
	if ( res != 1 )
		error ( 0, EINVAL, "indirizzo ip non valido" );
	return res;
}

int isPortValid ( char *argument, int inf, int sup ) {
	int i = 0;
	int res =  1;

	if ( argument[i] == '-') {
		error ( 0, EINVAL, "inserire un valore compreso tra: %d e %d", inf, sup );
		res = -1;
	} else {

		while ( res > 0 && argument[i] != '\0') {
			if ( isdigit ( argument[i] ) == 0 ) {
				error ( 0, EINVAL, "parametro non valido" );
				res = -1;
			}
			i++;
		}
	}
	if ( res > 0 ) {
		int port = atoi ( argument );
		if ( port < inf || port > sup ) {
			error ( 0, EINVAL, "inserire un valore compreso tra: %d e %d", inf, sup );
			res = -1;
		}
	}
	return res;
}

void clientInputCheck ( int argc, char **argv ) {
	char argv_err0[] 	= "e' necessario inserire: indirizzo IPv4 valido; numero di Porta\n";
	if ( argv[1] == NULL || argv[2] == NULL ) {
		write ( STDERR_FILENO, argv_err0, strlen( argv_err0 ) );
		exit( -1 );
	}
	
	int i = 0;

	//uso la funzione error per:
	//1 ) scrivere su standard error
	//2 ) richiamare un messaggio di errore standard

	int test;
	/* controllo valore operatori attivi*/
	if (( test = isValidIpAddress ( argv[1] ) == -1 ) ) {
		exit( -1 );
	}
	/* controllo valore oggetti da assegnare */	
	if (( test = isPortValid ( argv[2], 1024, 49150 ) == -1 ) ) {
		exit( -1 );
	}

}


/*===========================================================================*/
/*
3 ) operazioni su client
	riconosci comando
*/
/*===========================================================================*/


void showMenu () {

	char buff[256];
	int check = 0;
    char menu[]			= "ELENCO COMANDI:";
    char elenca[]		= "elenca:\n\n	Elenca le informazioni degli oggetti che l'operatore deve ancora gestire";
    char elserver[]		= "elencaserver:\n\n 	Richiede al SERVER l'elenco completo degli articoli";
    char consegnato[]	= "consegnato#codice:\n\n	Informa il SERVER della consegna ed elimina l'articolo dall'elenco locale";
    char ritirato[] 	= "ritirato#codice#descrizione#indirizzo:\n\n 	Informa il SERVER del ritiro di un nuovo oggetto ed aggiunge l'articolo nell'elenco locale";
    char smista[] 		= "smista#codice:\n\n 	Informa il SERVER della consegna in magazzino ed elimina l'articolo dall'elenco locale";

	char string[1204];
	sprintf ( string, "\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n", menu, elenca, elserver, consegnato, ritirato, smista );
	write ( STDOUT_FILENO, string, strlen ( string ) );
}



int commandToHash ( char *command ) {
	int i = 0;
	int k = 0;
	while ( i < 5 && ( strcasecmp ( command, cliCommands[i++] ) != 0 ) )
		k++;
		//printf ( "k: %d", k );
	return k;
}

void splitCommand ( char *string, const char *strbuffer ) {
	int i = 0;
	while ( strbuffer[i] != '\0' && strbuffer[i] != '\n' && strbuffer[i] != '#') {
		if ( strbuffer[i] == '#' || strbuffer[i] == '\n')
			string[i] = '\0';
		else
			string[i] = strbuffer[i++];
	}
	
}

int getCommand ( int inputFD, char **cmdPointer ) {

	int rVar = 1;
	int command;
	char *test;
	char *string = ( char *) malloc ( 256 * sizeof ( char ) );
	memset ( string, 0, strlen ( string ) );
	char *strbuffer = ( char *) malloc ( 256 * sizeof ( char ) );
	memset ( strbuffer, 0, strlen ( string ) );		

	if ( ( rVar = readLine ( inputFD, strbuffer ) ) > 0 ) {
		splitCommand ( string, strbuffer );
		command = commandToHash ( string );
		sprintf ( strbuffer, "%s\n", strbuffer );
		*cmdPointer = strbuffer;
	} else
		command = -1;
	return command;
}

/*===========================================================================*/
/*
4 ) operazioni network / thread
	
*/
/*===========================================================================*/

/*inizializza il socket di comunicazione del server*/
int InitServerSocket ( struct sockaddr_in *server, int port, int maxOperatorsQueue ) {

	//creo il socket
	int sockfd = socket( AF_INET , SOCK_STREAM , 0 );
	char mess[] = "Socket creato...\n";
	if ( sockfd == -1 )
	{
		char mess[] = "Non è possibile creare il socket";
		write( STDOUT_FILENO, mess, strlen ( mess ) );
	}
	write( STDOUT_FILENO, mess, strlen ( mess ) );
	
	//preaparo la struttura sockaddr_in che contenga le informazioni di connessione
	server = ( struct sockaddr_in *) malloc ( sizeof ( struct sockaddr_in ) );
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = INADDR_ANY;
	server->sin_port = htons ( port );
	
	//effettuo la Bind e gestisco l'eventuale errore
	//questo ciclo tenta la bind nel caso in cui non la porta non sia libera
	int bindErr;

	//rende socket non bloccante
	//fcntl( sockfd, F_SETFL, O_NONBLOCK );

	do {
		bindErr = bind( sockfd,( struct sockaddr *)server , sizeof( struct sockaddr_in ) );
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

/*===========================================================================*/


/*===========================================================================*/

char *encodePkgForTransmission ( Package *handler ) {

	if ( handler != NULL ) {
		char *message = ( char *) malloc ( 256 * sizeof ( char ) );
		//memset ( message, '\0', strlen ( message ) -1 );

		sprintf ( message, "%s#%s#%s#%d\n", handler->codice_articolo,
				handler->descrizione_articolo,
				handler->indirizzo_destinazione,
				handler->stato_articolo );	

		return message;
	}
}		

/*===========================================================================*/

char *decodePkgfromTransmission ( char *strbuffer ) {
	int tokensNumber = 4;
	char *str[tokensNumber];
	int i;
	int check = 1;
	char *ptr;
	for ( i = 0; i < tokensNumber; i++) {
		str[i] = ( char *) malloc ( 256 * sizeof ( char ) );
		if ( str[i] == NULL )
			perror ( "memoria esaurita" ), exit ( -1 );
		memset ( str[i], '\0', strlen ( str[i] ) );	
	}
	getTokens ( str, strbuffer, tokensNumber );
	char status[24];
	Status stat = atoi ( str[3] );
		switch ( stat ) {
			case STORAGE: strcpy ( status, "STORAGE" ); break;
			case TOBEDELIVERED: strcpy ( status, "TOBEDELIVERED" ); break;
			case DELIVERED: strcpy ( status, "DELIVERED" ); break;
			case COLLECTED: strcpy ( status, "COLLECTED" ); break;
			default: strcpy ( status, "boh?" ); break;
		}
	sprintf ( strbuffer, "%s , %s , %s . STATO: %s\n", str[0], str[1], str[2], status );
	return strbuffer;
}