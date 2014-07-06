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
1) operazioni su lista puntata
pkg_print stampa la lista intera
pkg_malloc alloca memoria per la struttura package
pkg_enqueue inserisce una struttura package in coda
pkg_push inserisce una struttura package in testa
pkg_delete cancella una struttura package dalla lista
list_delete cancella la lista
pkg_find cerca una struttura package nella lista
*/
/*===========================================================================*/


/**questo main serve solo per compilare il file in standalone. va rimosso
	prima della consegna*//*
int main () {
	return 0;
}
*/
/*
per motivi di efficienza (velocita'/memoria) di stack e heap preferisco
 chiamare prima la visita ricorsiva sul nodo successivo e poi
 effettuare le operazioni sul nodo.
*/
void pkglist_print (Package *handler) {
	if (handler != NULL) {
		pkglist_print(handler->next);	
		pkg_print (handler);

	}
}

void pkg_print (Package *handler) {
	if (handler != NULL) {
		char *message = malloc (256 * sizeof (char));
		char status[20];
		memset (message, '\0', strlen (message));

		switch (handler->stato_articolo) {
			case STORAGE: strcpy (status, "STORAGE"); break;
			case TOBEDELIVERED: strcpy (status, "TOBEDELIVERED"); break;
			case DELIVERED: strcpy (status, "DELIVERED"); break;
			case COLLECTED: strcpy (status, "COLLECTED"); break;
			default: strcpy (status, "boh?"); break;
		}

		sprintf (message, "Item= %s , %s , %s . STATO: %s\n", handler->codice_articolo,
				handler->descrizione_articolo,
				handler->indirizzo_destinazione,
				status);	
		write (STDOUT_FILENO, message, strlen (message));
		free (message);
	}
}


/**
pkg_initialize
status: -1 		-> non aggiornare lo stato e leggi da buffer
status: [0,3]	-> modifica lo stato con uno di quelli disponibili
*/
Package * pkg_initialize (char **buffer, int status) {
	int i = 0;
	char *err00 = "err00: memoria esaurita\n";
	char *err01 = "err01: impossibile inizializzare il mutex\n";	
	Package * newPkg   = (Package *) malloc (sizeof (Package));
	//controllo allocazione memoria
	if (newPkg == NULL) {
		write (STDERR_FILENO, err00, strlen (err00));
		return NULL;
	}
	memset (newPkg, 0, sizeof (Package));

	strcpy (newPkg->codice_articolo, buffer[0]);
	strcpy (newPkg->descrizione_articolo, buffer[1]);
	strcpy (newPkg->indirizzo_destinazione, buffer[2]);
	if (status != -1)
		newPkg->stato_articolo = status;
	else {
		i = atoi (buffer[3]);
		newPkg->stato_articolo = i;	
	}
	newPkg->next = NULL;
	return newPkg;
}

Package * pkg_enqueue (Package * handler, char **buffer, int status) {
	if (handler == NULL){
		Package * nuovo_nodo = pkg_initialize (buffer, status);
		if (nuovo_nodo != NULL) {
			nuovo_nodo->next = handler;
			handler	= nuovo_nodo;
		}
	} else
		handler->next = pkg_enqueue (handler->next, buffer, status);

	return handler;
}


Package * pkg_push (Package * handler, char **buffer, int status) {
	Package * newPkg = pkg_initialize (buffer, status);
	if (handler != NULL)
		newPkg->next = handler;
	return newPkg;
}



Package * pkg_delete (Package * handler, char *buffer0){
	if (handler == NULL){

	} else if (strcmp (buffer0, handler->codice_articolo) == 0){
		Package * nodo = handler->next;
		free(handler);
		handler = nodo;
	} else {
		handler->next = pkg_delete (handler->next, buffer0);
	}

	return handler;
}

void list_delete (Package * handler){

	if (handler != NULL){
		list_delete (handler->next);
		handler->next = NULL;
		free (handler);

	}
}



/**funzione di ricerca base basata su codice articolo*/
Package * pkg_find (Package * handler, char *pkgCode) {

	Package * result = NULL;

	//rettifica <= pkgCode
	if (handler != NULL) {
		if (strcmp (pkgCode, handler->codice_articolo) == 0) {
			result = handler;

		} else {
			result = pkg_find (handler->next, pkgCode);
		}

	}

	return result;
}

/**funzione di ricerca basata sullo stato di magazzino dell'articolo*/
Package * getStoredPackage (Package * handler, int status) {

	Package * result = NULL;

	if (handler != NULL) {
		if (handler->stato_articolo == status) {
			result = handler;

		} else {
			result = getStoredPackage (handler->next, status);
		}
	}

	return result;
}

int isEndOfMessage (char *string) {

	return (strcmp (string, "EOM"));
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

Package * createList (Package *handler, int inputFD, int tokensNumber, int status) {
	char *str[tokensNumber];
	int i;
	int check = 1;
	char *ptr;
	for (i = 0; i < tokensNumber; i++) {
		str[i] = (char *) malloc (256);
		memset (str[i], '\0', strlen (str[i]));
	}
	char *strbuffer = malloc (256);	
	memset (strbuffer, 0, strlen (strbuffer));
	while (check != 0 && (readLine (inputFD, strbuffer)) > 0) {
		//funzione di libreria che cerca una sottostringa
		//in una sottostringa e restituisce il puntatore
		ptr = strstr (strbuffer, "EOM#");
		if (ptr == NULL) {
			getTokens (str, strbuffer, tokensNumber);
			/**implementazione con push su "pila"*/
			handler = pkg_push (handler, str, status);
			for (i = 0; i < tokensNumber; i++)
				memset (str[i], '\0', strlen (str[i]));
			memset (strbuffer, 0, strlen (strbuffer));
		} else
			check = 0;	
	}

	for (i = 0; i < tokensNumber; i++)
		free (str[i]);
	free (strbuffer);
	
	return handler;
}
/**
prende in ingresso una linea del file di testo e la salva in una stringa
puntata.
la funzione restituisce il numero di lettere lette nella linea per controllare
che il file non sia terminato.
*/
int readLine (int inputFD, char *strbuffer) {
	memset (strbuffer, 0, strlen (strbuffer));
	int	i = 0;
	char c;
	while ((read (inputFD, &c, 1)) > 0 && (c != '\n') ) {
		//if (c == '\r')
		//	c = '\0';
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
void getTokens (char *string[], char *strbuffer, int tokensNumber) {
	int i = 0;
	int numString = 0;
	int k = 0;

	
	for (i = 0; i < tokensNumber -1 ; i++) {
		strbuffer = getSubstr (string[i], strbuffer, '#', 1);
	}
	
	getSubstr (string[i], strbuffer, '\0', 0);		
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
char *getSubstr (char *result, char *input, char terminal, int stepup) {
	int i = 0;
	char *punt;
	
	while (input[i] != terminal) {
		result[i] = input[i++];
	}
	if (stepup == 1) 
		input++;

	return input+i;
}

/*===========================================================================*/
/*
2) operazioni su command line arguments
*/
/*===========================================================================*/

int checkArguments (char *argument, int argNum) {
	int i 	= 0;
	int res = 1;

	if (argument[i] == '-') {
		error (0, EINVAL, "argomento %d -> inserire un numero positivo", argNum);
		res = -1;
	} else {

		while (res > 0 && argument[i] != '\0') {
			if (isdigit (argument[i]) == 0) {
				error (0, EINVAL, "argomento %d -> inserire un numero", argNum);
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

int serverInputCheck (int argc, char **argv) {
	int i = 0;
	int test;
	char argv_err0[] = "e' necessario inserire: N operatori; K oggetti; file di lettura\n";
	if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL) {
		write (STDERR_FILENO, argv_err0, strlen(argv_err0));
		exit(-1);
	}
	

	//uso la funzione error per:
	//1) scrivere su standard error
	//2) richiamare un messaggio di errore standard

	/* controllo valore operatori attivi*/
	if ((test = checkArguments (argv[1], 1)) < 0) {
		exit(-1);
	}
	/* controllo valore oggetti da assegnare */	
	if ((test = checkArguments (argv[2], 2)) < 0) {
		exit(-1);
	}

	/* controllo file in lettura*/
	/* ENOENT = No such file or directory */
	test = open (argv[3], O_RDONLY); 
	if (test < 0)
		error (0, ENOENT , "file di testo non valido"), exit (-1);


	if (argv[4] != NULL && argv[4][1] == '-' && argv[4][1] == 'd')
		DEBUG = 1;

	return test;
}


int randomIntGenerator (int inf, int sup) {

	return rand()%(sup - inf +1) + inf;
}



/*===========================================================================*/
/*
3) operazioni su clientinput
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
-1 errore (imposta errno, recuperato da perror e stampato su standard error)
*/

int isValidIpAddress (char *ipAddress) {
	struct sockaddr_in test;
	int res = inet_pton(AF_INET, ipAddress, &(test.sin_addr));
	if (res != 1)
		error (0, EINVAL, "indirizzo ip non valido");
	return res;
}

int isPortValid (char *argument, int inf, int sup) {
	int i = 0;
	int res =  1;

	if (argument[i] == '-') {
		error (0, EINVAL, "inserire un valore compreso tra: %d e %d", inf, sup);
		res = -1;
	} else {

		while (res > 0 && argument[i] != '\0') {
			if (isdigit (argument[i]) == 0) {
				error (0, EINVAL, "parametro non valido");
				res = -1;
			}
			i++;
		}
	}
	if (res > 0) {
		int port = atoi (argument);
		if (port < inf || port > sup) {
			error (0, EINVAL, "inserire un valore compreso tra: %d e %d", inf, sup);
			res = -1;
		}
	}
	return res;
}

void clientInputCheck (int argc, char **argv) {
	char argv_err0[] 	= "e' necessario inserire: indirizzo IPv4 valido; numero di Porta\n";
	if (argv[1] == NULL || argv[2] == NULL) {
		write (STDERR_FILENO, argv_err0, strlen(argv_err0));
		exit(-1);
	}
	
	int i = 0;

	//uso la funzione error per:
	//1) scrivere su standard error
	//2) richiamare un messaggio di errore standard

	int test;
	/* controllo valore operatori attivi*/
	if ((test = isValidIpAddress (argv[1]) == -1)) {
		exit(-1);
	}
	/* controllo valore oggetti da assegnare */	
	if ((test = isPortValid (argv[2], 1024, 49150) == -1)) {
		exit(-1);
	}

}


/*===========================================================================*/
/*
3) operazioni su client
	riconosci comando
*/
/*===========================================================================*/


void showMenu () {
    char menu[]			= "ELENCO COMANDI:\n\n";
    char elenca[]		= "elenca:\n	Elenca le informazioni degli oggetti che l'operatore deve ancora gestire\n";
    char elserver[]		= "elencaserver:\n 	Richiede al SERVER l'elenco completo degli articoli\n";
    char consegnato[]	= "consegnato#codice:\n	Informa il SERVER della consegna ed elimina l'articolo dall'elenco locale\n";
    char ritirato[] 	= "ritirato#codice#descrizione#indirizzo:\n 	Informa il SERVER del ritiro di un nuovo oggetto ed aggiunge l'articolo nell'elenco locale\n";
    char smista[] 		= "smista#codice:\n 	Informa il SERVER della consegna in magazzino ed elimina l'articolo dall'elenco locale\n";
}

void commandSwitch (int command, Package *handler, int sockfd) {
	char *err01 = "warning! comando non valido!\n";

	switch (command) {

		case ELENCASERVER:
		/*ELENCA STAMPA LA LISTA REMOTA*/
			write (STDOUT_FILENO, "elencaserver\n", strlen ("elencaserver\n"));
			elencaClientToServer (sockfd);
		break;
		case CONSEGNATO:
			write (STDOUT_FILENO, "consegnato\n", strlen ("consegnato\n"));
		break;
		case RITIRATO:
			write (STDOUT_FILENO, "ritirato\n", strlen ("ritirato\n"));
		break;
		case SMISTA:
			write (STDOUT_FILENO, "smista\n", strlen ("smista\n"));
		break;
		case ELENCA:
		/*ELENCA STAMPA LA LISTA LOCALE*/
			write (STDOUT_FILENO, "elenca:\n", strlen ("elenca:\n"));
			pkglist_print (handler);
		break;						
		default:
			write (STDOUT_FILENO, err01, strlen (err01));
		break;
	}
}

int commandToHash (char *command, char **cliCommands) {
	int i = 0;
	int k = 0;
	while ( i < 5 && (strcasecmp (command, cliCommands[i++]) != 0))
		k++;
		//printf ("k: %d", k);
	return k;
}

void getCommand (char *string, const char *strbuffer) {
	int i = 0;
	while (strbuffer[i] != '\0' && strbuffer[i] != '\n' && strbuffer[i] != '#') {
		if (strbuffer[i] == '#' || strbuffer[i] == '\n')
			string[i] = '\0';
		else
			string[i] = strbuffer[i++];
	}
	
}
int getLine (int inputFD, char **cliCommands) {

	char *string = (char *) malloc (256);
	memset (string, '\0', strlen (string));	

	int rVar = 1;
	int command;
	char *strbuffer = malloc (256);	
	memset (strbuffer, 0, strlen (strbuffer));

	if ( (rVar = readLine (inputFD, strbuffer))> 0 ) {

		//output per debug
		write (STDOUT_FILENO, strbuffer, strlen (strbuffer));
		write (STDOUT_FILENO, "\n", 1);
		
		getCommand (string, strbuffer);
		command = commandToHash (string, cliCommands);
		//debug
		//write (STDOUT_FILENO, string, strlen (string));
	}
	return command;
}

/*===========================================================================*/
/*
4) operazioni network / thread
	
*/
/*===========================================================================*/

/*inizializza il socket di comunicazione del server*/
int InitServerSocket (struct sockaddr_in *server, int port, int maxOperatorsQueue) {

	//creo il socket
	int sockfd = socket(AF_INET , SOCK_STREAM , 0);
	char mess[] = "Socket creato";
	if (sockfd == -1)
	{
		char mess[] = "Non è possibile creare il socket";
		write(STDOUT_FILENO, mess, strlen (mess));
	}
	write(STDOUT_FILENO, mess, strlen (mess));
	
	//preaparo la struttura sockaddr_in che contenga le informazioni di connessione
	server = (struct sockaddr_in *) malloc (sizeof (struct sockaddr_in));
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = INADDR_ANY;
//	server->sin_port = htons( 8888 );
 	if (port == -1)
		server->sin_port = htons( 8888 );
	else
		server->sin_port = htons ( port );
	
	//effettuo la Bind e gestisco l'eventuale errore
	//questo ciclo tenta la bind nel caso in cui non la porta non sia libera
	int bindErr;

	//rende socket non bloccante
	//fcntl(sockfd, F_SETFL, O_NONBLOCK);

	do {
		bindErr = bind(sockfd,(struct sockaddr *)server , sizeof(struct sockaddr_in));
		int errsv = errno;
		if (bindErr < 0) {
			if ( /* errsv == EINVAL || */ errsv == EADDRINUSE )
				perror ("Bind > Error3: Nuovo tentativo di assegnazione tra 5 secondi"), sleep (5);
			else
				perror ("Bind > Error4: exiting...."), exit (-1);
		} else if (bindErr == 0)
			write (STDOUT_FILENO, "Bind eseguita.\n", strlen ("Bind eseguita.\n"));  
	} while (bindErr < 0);

	//avvio la Listen su un numero N di operatori
	int listErr = listen(sockfd , maxOperatorsQueue);
	if (listErr == -1)
		perror ("Errore di listen"), exit (-1);

	char portH[6];
	sprintf (portH, "%d\n", port);
	char *message = "In attesa di connessioni...su porta: ";
	write (STDOUT_FILENO, message, strlen (message));
	write (STDOUT_FILENO, portH, strlen (portH));

	return sockfd;
}

/*===========================================================================*/


int initClientSocket (char **argv) {

	char *address;
	struct addrinfo hints, *res;
	memset (&hints, 0, sizeof (hints));
	char *mess00 = "ip agganciato...\n";
	char *mess01 = "socket creato...\n";
	char *mess02 = "connessione eseguita.\n";
	char *mess03 = "impossibile connettersi.\n";
	hints.ai_family		= AF_INET; //protocollo ipv4
	hints.ai_socktype   = SOCK_STREAM; //tcp
	hints.ai_flags		= AI_PASSIVE; //INADDR_ANY
	int sockfd;
		//argv[1] = ip; argv[2] = port
	if (getaddrinfo (argv[1], argv[2], &hints, &res) == -1)
		perror ("Error1: "), exit (-1);
	write (STDOUT_FILENO, mess00, strlen (mess00));
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd == -1)
		perror ("Error2: "), exit (-1);
	write (STDOUT_FILENO, mess01, strlen (mess01));

	int bindErr;
	int timeout = 12;
	while ((connect (sockfd, res->ai_addr, res->ai_addrlen) < 0) && timeout > 0) {
		perror ("connect >  Nuovo tentativo di connessione tra 5 secondi"), sleep (5);
		timeout++;
	}
	//se il timeout scade il programma chiude
	if (timeout == 0)
		write (STDERR_FILENO, mess03, strlen (mess03)), exit (-1);
	
	write (STDOUT_FILENO, mess02, strlen (mess02));
	return sockfd;
}

/*===========================================================================*/

char *encodePkgForTransmission (Package *handler) {

	if (handler != NULL) {
		char *message = malloc (256 * sizeof (char));
		//memset (message, '\0', strlen (message) -1);

		sprintf (message, "Item= %s#%s#%s#%d\n", handler->codice_articolo,
				handler->descrizione_articolo,
				handler->indirizzo_destinazione,
				handler->stato_articolo);	

		return message;
	}
}		

/*===========================================================================*/


void threadClientInit (int sockfd, Package *handler, int kPackages) {

	int i = 0;
	char *message;
	Package *current = handler;
	if (current == NULL) {
		while (current == NULL);
	}
	pthread_mutex_lock (&packageMutex);
	while (current != NULL && i < kPackages) {
		if (current->stato_articolo == STORAGE) {
			//leggi pacchetto
			current->stato_articolo = TOBEDELIVERED;
			pkg_print (current);
			//codifica e invia pacchetto
			message = encodePkgForTransmission (current);
			write (sockfd, message, strlen (message));
			current = current->next;
			i++;
			free (message);
			//usleep (500000);
			usleep (50000);
		} else {
			current = current->next;
		}
	}
	pthread_mutex_unlock (&packageMutex);
	write (sockfd, "EOM#\n", sizeof ("EOM#\n"));
}

void *connection_handler (void *parametri) {
	signal(SIGPIPE ,SIG_IGN);
	Passaggio *tmp = (Passaggio *)parametri;
	int err0;
	int err;
	int i = 0;
	int client_sock = tmp->sockfd;
	int kPackages = tmp->kPackages;
	Package *handler = tmp->handler;
	threadClientInit (client_sock, handler, kPackages);

	perror ("socket client: ");
	write (STDOUT_FILENO, "\nfine comunicazioni\n", sizeof ("\nfine comunicazioni\n"));	

	/*
	all'uscita del thread, lo elimino dalla "coda" dei thread attivi e libero
	uno slot per la connessione. impiego il mutex per poter scrivere sulla
	variabile globale
	*/
	pthread_mutex_lock (&maxThreadsMutex);
	maxThread--;
	pthread_mutex_unlock (&maxThreadsMutex);

	return((void *)0); 
}
