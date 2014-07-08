#include "library.h"

void threadClientInit (int sockfd, Package *handler, int kPackages) {
	write (STDOUT_FILENO, "Init iniziata\n", 14);
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
			//usleep (50000);
		} else {
			current = current->next;
		}
	}
	pthread_mutex_unlock (&packageMutex);
	write (sockfd, "EOM#\n", sizeof ("EOM#\n"));
	write (STDOUT_FILENO, "Init Terminata\n", 15);
}

/*THREAD CONNESSIONE*/
void *thread_connection_handler (void *parametri) {
	signal(SIGPIPE ,SIG_IGN);
	Passaggio *tmp = (Passaggio *)parametri;
	int err0;
	int err;
	int i = 0;
	int client_sock = tmp->sockfd;
	int kPackages = tmp->kPackages;
	Package *handler = tmp->handler;
	threadClientInit (client_sock, handler, kPackages);
	talkWithClient(client_sock, handler);
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

void talkWithClient (int client_sock, Package *handler) {
	int command = 0;
	while (command != -1) {
		write (STDOUT_FILENO, "\nmessaggio in arrivo:\n", 22);
		char *strbuffer;
		command = getLine (client_sock, &strbuffer);
		//output per debug
		if (strbuffer != NULL) {
			write (STDOUT_FILENO, "getline: ", sizeof ("getline: "));		
			write (STDOUT_FILENO, strbuffer, strlen (strbuffer));
			write (STDOUT_FILENO, "\n", 1);	
			command  = commandSwitchServer (command, strbuffer, handler, client_sock);
			write (STDOUT_FILENO, "\nmessaggio terminato\n", 20);
		}
	}

}


int commandSwitchServer (int command, char *cmdPointer,
                          Package *handler, int sockfd) {
	char *err01 = "warning! comando non valido!\n";

	switch (command) {
		case -1:
		command = -1;
		break;
		case ELENCASERVER:
		/*ELENCA STAMPA LA LISTA REMOTA*/
			write (STDOUT_FILENO, "elencaserver\n", strlen ("elencaserver\n"));
			elencaserver_server (sockfd, handler);
		break;
		case CONSEGNATO:
			write (STDOUT_FILENO, "consegnato\n", strlen ("consegnato\n"));
		break;
		case RITIRATO:
			write (STDOUT_FILENO, "ritirato\n", strlen ("ritirato\n"));
			ritirato_server (sockfd, cmdPointer, handler);
			//pkglist_print (handler);
		break;
		case SMISTA:
			write (STDOUT_FILENO, "smista\n", strlen ("smista\n"));
		break;					
		default:
			write (STDOUT_FILENO, err01, strlen (err01));
		break;
	}
	return command;
}


void elencaserver_server (int sockfd, Package *handler) {

	int i = 0;
	char *message;
	Package *current = handler;
	if (current == NULL) {
		while (current == NULL);
	}
	pthread_mutex_lock (&packageMutex);
	while (current != NULL) {
		
			//leggi pacchetto
			//pkg_print (current);
			//codifica e invia pacchetto
			message = encodePkgForTransmission (current);
			write (sockfd, message, strlen (message));
			i++;
			free (message);
			//usleep (500000);
			//usleep (50000);
			current = current->next;
	}
	pthread_mutex_unlock (&packageMutex);
	write (sockfd, "EOM#\n", sizeof ("EOM#\n"));

}

void ritirato_server (int sockfd, char *strbuffer, Package *handler) {
	write (STDOUT_FILENO, "articolo in ritiro: ", sizeof ("articolo in ritiro: "));		
	write (STDOUT_FILENO, strbuffer, strlen (strbuffer));
	write (STDOUT_FILENO, "\n", 1);	
	int tokensNumber = 3;
	char *str[tokensNumber];
	int i;
	int check = 1;
	char *ptr;
	int lenght = strlen (strbuffer);
	strbuffer[lenght-1] = '\0';
	for (i = 0; i < tokensNumber; i++) {
		str[i] = (char *) malloc (256 * sizeof (char));
		memset (str[i], 0, strlen (str[i]));
	}
	Status status = COLLECTED;
	getTokens (str, &strbuffer[9], tokensNumber);
	
/**implementazione con push su "pila"*/
/**/	
pthread_mutex_lock (&packageMutex);
	handler = pkg_enqueue (handler, str, status);
	pthread_mutex_unlock (&packageMutex);
	memset (strbuffer, 0, strlen (strbuffer));
	for (i = 0; i < tokensNumber; i++)
		free (str[i]);
	free (strbuffer);
/**/
}

//handler = createList (handler, sockfd,  4, -1)