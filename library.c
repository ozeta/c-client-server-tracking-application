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
		char *sepEQ = " = ";
		char *sepComma = " , ";
		char *sepCR = "\n";
		char *message = malloc (256);
		char *status = malloc (14);
		memset (status, '\0', 14);
		memset (message, 0, strlen (message));
		message = strcat (message, "Item");
		strcat (message, sepEQ);
		strcat (message, handler->codice_articolo);
		strcat (message, sepComma);
		strcat (message, handler->descrizione_articolo);
		strcat (message, sepComma);
		strcat (message, handler->indirizzo_destinazione);
		strcat (message, sepComma);
		strcat (message, "STATO: ");
		switch (handler->stato_articolo) {
			case STORAGE: status = "STORAGE"; break;
			case TOBEDELIVERED: status = "TOBEDELIVERED"; break;
			case DELIVERED: status = "DELIVERED"; break;							
			case COLLECTED: status = "COLLECTED"; break;
			default: status = "boh?"; break;
		}
		strcat (message, status);
		strcat (message, sepCR);		
		write (STDOUT_FILENO, message, strlen (message));
		free (message);
	}
}


/**
pkg_initialize
status: -1 		-> non aggiornare lo stato
status: [0,3]	-> modifica lo stato con uno di quelli disponibili
*/
Package * pkg_initialize(char *buffer0, char *buffer1, char *buffer2, Status status){
	char *err00 = "err00: memoria esaurita\n";
	char *err01 = "err01: impossibile inizializzare il mutex\n";	
	Package * newPkg   = (Package *) malloc (sizeof (Package));
	//controllo allocazione memoria
	if (newPkg == NULL) {
		write (STDERR_FILENO, err00, strlen (err00));
		return NULL;
	}
	memset (newPkg->codice_articolo, '\0', strlen(newPkg->codice_articolo));
	memset (newPkg->descrizione_articolo, '\0', strlen(newPkg->descrizione_articolo));
	memset (newPkg->indirizzo_destinazione, '\0', strlen(newPkg->indirizzo_destinazione));
	strcpy (newPkg->codice_articolo, buffer0);
	strcpy (newPkg->descrizione_articolo, buffer1);
	strcpy (newPkg->indirizzo_destinazione, buffer2);
	if (status != -1)
		newPkg->stato_articolo = status;
	newPkg->next = NULL;
	return newPkg;
}

Package * pkg_enqueue (Package * handler, char *buffer0, char *buffer1, char *buffer2, Status status) {
	if (handler == NULL){
		Package * nuovo_nodo = pkg_initialize (buffer0, buffer1, buffer2, status);
		if (nuovo_nodo != NULL) {
			nuovo_nodo->next = handler;
			handler	= nuovo_nodo;
		}
	} else
		handler->next = pkg_enqueue (handler->next, buffer0, buffer1, buffer2, status);

	return handler;
}


Package * pkg_push (Package * handler, char *buffer0, char *buffer1, char *buffer2, Status status) {
	Package * newPkg = pkg_initialize (buffer0, buffer1, buffer2, status);
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
Package * pkg_find (Package * handler, char *input) {

	Package * result = NULL;

	//rettifica <= input
	if (handler != NULL) {
		if (strcmp (input, handler->codice_articolo) == 0) {
			result = handler;

		} else {
			result = pkg_find (handler->next, input);
		}

	}

	return result;
}

/**funzione di ricerca basata sullo stato di magazzino dell'articolo*/
Package * getStoredPackage (Package * handler, Status input) {

	Package * result = NULL;

	if (handler != NULL) {
		if (handler->stato_articolo == input) {
			result = handler;

		} else {
			result = getStoredPackage (handler->next, input);
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
Package * createList(Package *handler, int inputFile, int tokensNumber, Status status) {
	char *string[tokensNumber];
	int i = 0;

	for (; i < tokensNumber; i++) {
		string[i] = (char *) malloc (256);
		memset (string[i], '\0', strlen (string[i]));
	}

	char *strbuffer = malloc (256);	
	memset (strbuffer, 0, strlen (strbuffer));

	while ( (readLine (inputFile, strbuffer))> 0 ) {
/*
		output per debug
		write (STDOUT_FILENO, strbuffer, strlen (strbuffer));
		write (STDOUT_FILENO, "\n", 1);
*/		
		getTokens (string, strbuffer, tokensNumber);
/*
		debug
		memset (strbuffer, '\0', strlen (strbuffer));
		write (STDOUT_FILENO, string[0], strlen (string[0]));
		write (STDOUT_FILENO, "\n", 1);
		output per debug
		printf ("%s -> %s -> %s\n", string[0], string[1], string[2]);
*/

		/**implementazione in coda*/
		//handler = pkg_enqueue (handler, string[0], string[1], string[2], status);
		/**implementazione su pila*/
		handler = pkg_push (handler, string[0], string[1], string[2], status);
		for (i = 0; i < tokensNumber; i++)
			memset (string[i], '\0', strlen (string[i]));

	}
	write (STDOUT_FILENO, "\n", 1);
	close (inputFile);

	for (; i < tokensNumber; i++)
		free (string[i]);
	free (strbuffer);
	
	return handler;
}

/**
prende in ingresso una linea del file di testo e la salva in una stringa
puntata.
la funzione restituisce il numero di lettere lette nella linea per controllare
che il file non sia terminato.
*/
int readLine (int inputFileDes, char *strbuffer) {

	int	i = 0;
	char c;
	while ((read (inputFileDes, &c, 1)) > 0 && (c != '\n') ) {
		//if (c == '\r')
		//	c = '\0';
		strbuffer[i++] = c;
	}
	strbuffer[i] = '\0';	

	return i;

}

/**
la procedura prende in ingresso il puntatore all'array di stringhe,
il puntatore alla stringa temporanea.
il numero di token da leggere
per ognuno dei 3 token da leggere, la funzione chiama getTocken.
*/
void getTokens (char *string[], char *strbuffer, int tokensNumber) {
	int i = 0;
	int numString = 0;
	int k = 0;

	
	for (i = 0; i < tokensNumber -1 ; i++) {
		strbuffer = getToken (string[i], strbuffer, '#', 1);
	}

	getToken (string[2], strbuffer, '\0', 0);		
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
char *getToken (char *result, char *input, char terminal, int stepup) {
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
2) operazioni su input
*/
/*===========================================================================*/

int checkArguments (char *argument, int argNum) {
	int i = 0;
	int res =  1;

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
	char argv_err0[] 	= "e' necessario inserire: N operatori; K oggetti; file di lettura\n";
	if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL) {
		write (STDERR_FILENO, argv_err0, strlen(argv_err0));
		exit(-1);
	}
	
	int i = 0;

	//uso la funzione error per:
	//1) scrivere su standard error
	//2) richiamare un messaggio di errore standard

	int test;
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
void commandSwitch (int command, Package *handler) {
	char *err01 = "warning! comando non valido!\n";

	switch (command) {

		case ELENCASERVER:
		/*ELENCA STAMPA LA LISTA REMOTA*/
			write (STDOUT_FILENO, "elencaserver\n", strlen ("elencaserver\n"));
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
	while ( i < 5 && (strcmp (command, cliCommands[i++]) != 0))
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


int InitSocket (struct sockaddr_in *server, int port, int maxOperatorsQueue) {

	//creo il socket
	int sockfd = socket(AF_INET , SOCK_STREAM , 0);

	if (sockfd == -1)
	{
		char mess[] = "Non è possibile creare il socket";
		write(STDOUT_FILENO, mess, strlen (mess));
	}
	puts("Socket creato");
	 
	//ma il server che indirizzo ip deve avere?!
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

int connectToServer (char **argv) {

	char *address;
	struct addrinfo hints, *res;
	memset (&hints, 0, sizeof (hints));
	char *mess00 = "ip agganciato...\n";
	char *mess01 = "socket creato...\n";
	char *mess02 = "connessione eseguita.\n";
	char *mess03 = "impossibile connettersi.\n";
	hints.ai_family	 = AF_INET; //protocollo ipv4
	hints.ai_socktype   = SOCK_STREAM; //tcp
	hints.ai_flags	  = AI_PASSIVE; //INADDR_ANY
	int sockfd;
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
	if (timeout == 0)
		write (STDERR_FILENO, mess03, strlen (mess03)), exit (-1);
	
	write (STDOUT_FILENO, mess02, strlen (mess02));
	return sockfd;
}

/*===========================================================================*/

char *encodePkgForTransmission (int sockfd, Package *handler) {

	if (handler != NULL) {
		char *sepComma = "#";
		char *sepCR = "\n";
		char *message = malloc (256);
		char *status = malloc (14);
		memset (status, '\0', 14);
		memset (message, 0, strlen (message));
		strcat (message, handler->codice_articolo);
		strcat (message, sepComma);
		strcat (message, handler->descrizione_articolo);
		strcat (message, sepComma);
		strcat (message, handler->indirizzo_destinazione);
		strcat (message, sepComma);

		switch (handler->stato_articolo) {
			case STORAGE: status = "STORAGE"; break;
			case TOBEDELIVERED: status = "TOBEDELIVERED"; break;
			case DELIVERED: status = "DELIVERED"; break;							
			case COLLECTED: status = "COLLECTED"; break;
			default: status = "boh?"; break;
		}
		strcat (message, status);
		strcat (message, sepCR);		
		write (sockfd, message, strlen (message));
	}

}

/*===========================================================================*/



initClient (int sock, Package *handler, int kPackages) {
	//ATTENZIONE, QUI BISOGNA GESTIRE IL MUTEX GLOBALE!!!
	int i = 0;

	Package *current = handler;
	if (current == NULL) {
		while (current == NULL);
	}
	//BLOCCA MUTEX
	while (current != NULL && i < kPackages) {
		if (current->stato_articolo == STORAGE) {
			//leggi pacchetto
			current->stato_articolo = TOBEDELIVERED;
			//codifica e invia pacchetto

			current = current->next;
			i++;
		} else {
			current = current->next;
		}
	}
	write (sock, "INIT_END#", sizeof ("INIT_END#"));
	//SBLOCCA MUTEX
}

void *connection_handler (void *parametri) {
	//pthread_t tid = pthread_self();
	//char buff[10];
	//sprintf (buff, "%d", (int) tid);
	//write (STDOUT_FILENO, buff, strlen (buff));
	//write (STDOUT_FILENO, "\n", 1);
	int client_sock = *(int *)parametri;
	int err0;
	int i = 0;
	while ( i < 10) {
		//write (STDOUT_FILENO, "working ", sizeof ("working "));		
		//write (client_sock, "A", 1);
		(err0 = write (client_sock, "A", 1));
		//sleep (1);
		i++;		
	}
	perror ("socket client: ");
	write (STDOUT_FILENO, "\nfine comunicazioni\n", sizeof ("\nfine comunicazioni\n"));	
	/*
	Passaggio *tmp = (Passaggio *)parametri;
	int sock = tmp->sockfd;
	int kPackages = tmp->kPackages;
	Package *handler = tmp->handler;
	
	initClient (sock, handler, kPackages);
	*/
	int err = pthread_mutex_lock (&maxThreadsMutex);
	if (err != 0)
		perror ("mutex: "), exit (-1);
	maxThread--;
	char message[20];
	sprintf ( message,"nuovo maxThread: %d\n", maxThread);
	write(STDERR_FILENO , message , strlen(message));	
	err = pthread_mutex_unlock (&maxThreadsMutex);
	if (err != 0)
		perror ("mutex: "), exit (-1);
	return 0;
}


