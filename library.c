#include "library.h"
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
void pkglist_print (Package * handler){
    if (handler != NULL){
		char *sep0 = " = ";
		char *sep = " , ";
		char *message = "handler->val";
		message = strcat (message, sep0);
		message = strcat (message, handler->codice_articolo);
		message = strcat (message, sep);
		message = strcat (message, handler->descrizione_articolo);
		message = strcat (message, sep);
		message = strcat (message, handler->indirizzo_destinazione);		
		write (STDOUT_FILENO, message, strlen (message));


        pkglist_print(handler->next);
    }else
        printf("\n");
}

Package * pkg_malloc(char *buffer0, char *buffer1, char *buffer2){

    Package * newPkg   = (Package *) malloc (sizeof (Package));
	memset (newPkg->codice_articolo, '\0', strlen(newPkg->codice_articolo));
	memset (newPkg->descrizione_articolo, '\0', strlen(newPkg->descrizione_articolo));
	memset (newPkg->indirizzo_destinazione, '\0', strlen(newPkg->indirizzo_destinazione));
	strcpy (newPkg->codice_articolo, buffer0);
	strcpy (newPkg->descrizione_articolo, buffer1);
	strcpy (newPkg->indirizzo_destinazione, buffer2);	
	newPkg->stato_articolo = STORAGE;
	//newPkg->sem = ;
	newPkg->next = NULL;
    
	return newPkg;
}

Package * pkg_enqueue (Package * handler, char *buffer0, char *buffer1, char *buffer2){
    if (handler == NULL){
        Package * nuovo_nodo	= pkg_malloc (buffer0, buffer1, buffer2);
        nuovo_nodo->next   		= handler;
        handler = nuovo_nodo;
    } else
        handler->next = pkg_enqueue (handler->next, buffer0, buffer1, buffer2);

    return handler;
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

Package * pkg_find (Package * handler, char *buffer0, char *buffer1, char *buffer2) {

    Package * result = NULL;

	//rettifica <= input
    if (handler != NULL) {
        if (strcmp (buffer0, handler->codice_articolo) == 0) {
            result = handler;

        } else {
            result = pkg_find (handler->next, buffer0, buffer1, buffer2);
        }

    }

    return result;
}

char *getToken (char *result, char *input, char separator) {
	int i = 0;
	char *punt;
	while (input[i] != separator) {
		result[i] = input[i++];
	}
	
	return &input[i];
}

void getTokens (char *string[], char *strbuffer, int rVar, int tkNum) {
	int i = 0;
	int numString = 0;
	int k = 0;


	strbuffer = getToken (string[0], strbuffer, '#');
	strbuffer++;
	strbuffer = getToken (string[1], strbuffer, '#');
	strbuffer++;
	strbuffer = getToken (string[2], strbuffer, '\0');		
/*
		while (strbuffer[i] != '#') {
			string[numString][k++] = *(strbuffer+(i++));
		}

		numString++;
		i++;
		k = 0;
		while (strbuffer[i] != '#') {
			string[numString][k++] = *(strbuffer+(i++));
		}

		numString++;
		i++;
		k = 0;
		while (strbuffer[i] != '\0') {
			string[numString][k++] = *(strbuffer+(i++));
		}
*/

}
int getLineFromFile (int inputFile, char *strbuffer) {

	int	i 	 = 0;
	int rVar = 1;
	char c;
	while ((rVar = read (inputFile, &c, 1)) > 0 && c != '\n') {
		strbuffer[i++] = c;
	}
	strbuffer[--i] = '\0';	

	return i;

}
int createList(Package *handler, int inputFile, int tokensNumber) {
	char *string[tokensNumber];
	int i = 0;

	for (; i < tokensNumber; i++) {
		string[i] = (char *) malloc (256);
		memset (string[i], '\0', strlen (string[i]));
	}

	char *strbuffer = malloc (256);	
	memset (strbuffer, 0, strlen (strbuffer));

	int rVar = 1;
	int wVar;

	while ( (rVar = getLineFromFile (inputFile, strbuffer))> 0 ) {
		//output per debug
		//write (STDOUT_FILENO, strbuffer, strlen (strbuffer));
		//write (STDOUT_FILENO, "\n", 1);
		getTokens (string, strbuffer, rVar, tokensNumber);
		//memset (strbuffer, '\0', strlen (strbuffer));
		//write (STDOUT_FILENO, string[0], strlen (string[0]));
		//write (STDOUT_FILENO, "\n", 1);
		printf ("%s -> %s -> %s\n", string[0], string[1], string[2]);
		for (i = 0; i < tokensNumber; i++)
			memset (string[i], '\0', strlen (string[i]));

	}
	write (STDOUT_FILENO, "\n", 1);
	close (inputFile);

	for (; i < tokensNumber; i++)
		free (string[i]);
	free (strbuffer);
	
	return 0;
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

int serverInputCheck (int argc, char **argv) {
/**
Il processo server riceve su linea di comando:
	intero N che definisce il numero massimo di operatori attivi;
	intero K che definisce il numero di oggetti assegnati inizialmente ad ogni operatore;
	stringa S nome del file contenente la lista degli oggetti da consegnare
*/


	char argv_err0[] 	= "e' necessario inserire N operatori, K oggetti e il file di lettura\n";

	if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL) {
		write (STDERR_FILENO, argv_err0, strlen(argv_err0));
	//	write (STDERR_FILENO, argv_err0, sizeof(argv_err0));
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
int isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in test;
    int res = inet_pton(AF_INET, ipAddress, &(test.sin_addr));
	if (res == -1)
		perror ("indirizzo ip non valido"), exit (-1);
    return res;
	
}

/*===========================================================================*/
/*
4) operazioni network / thread
	
*/
/*===========================================================================*/


int InitSocket (struct sockaddr_in *server, int port, int operatorsNumber) {

    //creo il socket
    int sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1)
    {
        printf("Non è possibile creare il socket");
    }
    puts("Socket creato");
     
	//ma il server che indirizzo ip deve avere?!
    //preaparo la struttura sockaddr_in che contenga le informazioni di connessione
	server = (struct sockaddr_in *) malloc (sizeof (struct sockaddr_in));
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = INADDR_ANY;
//    server->sin_port = htons( 8888 );
 	if (port == -1)
		server->sin_port = htons( 8888 );
    else
		server->sin_port = htons ( port );
    
    //effettuo la Bind e gestisco l'eventuale errore
	//questo ciclo tenta la bind nel caso in cui non la porta non sia libera
    int bindErr;
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
    listen(sockfd , operatorsNumber);
	
	char *message = "In attesa di connessioni...su porta: ";
	//inserire numero porta
    write (STDOUT_FILENO, message, strlen (message));
  
	return sockfd;
}