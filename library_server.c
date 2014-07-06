#include "library.h"

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
	//while (1) {
		char *cmdPointer;
		memset (cmdPointer, 0, 256 * sizeof (char)); 	
		int command = getLine (client_sock, cmdPointer);
		commandSwitchServer (command, cmdPointer, handler, client_sock);
	//}

}


void commandSwitchServer (int command, char *cmdPointer,
                          Package *handler, int sockfd) {
	char *err01 = "warning! comando non valido!\n";

	switch (command) {

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
		break;
		case SMISTA:
			write (STDOUT_FILENO, "smista\n", strlen ("smista\n"));
		break;
		case ELENCA:
		/*ELENCA STAMPA LA LISTA LOCALE*/
			write (STDOUT_FILENO, "elenca:\n", strlen ("elenca:\n"));
		break;						
		default:
			write (STDOUT_FILENO, err01, strlen (err01));
		break;
	}
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
		if (current->stato_articolo == STORAGE) {
			//leggi pacchetto
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