/*=============================================================================
  Nome: server.c
  Autori: Marco Carrozzo, Maurizio Del Prete
  Progetto: Corriere Espresso
  ===========================================================================*/

/*
	compilazione:
	gcc server.c -lpthread -o server
*/

#include "library.h"

//funzione master thread
void *connection_handler (void *);



int main (int argc , char *argv[]) {
	//check preliminare su input
	int inputFD = serverInputCheck(argc, argv);
	int operatorsNumber = atoi (argv[1]);
	int kPackages		= atoi (argv[2]);


	int err = pthread_mutex_init (&maxThreadsMutex, NULL);
	if (err != 0) {
		perror ("Impossibile allocare il mutex"), exit (-1);
	}
	Package *handler	= NULL;

	int tokensNumber = 3;
	Status status = STORAGE;
	handler = (Package *)createList (handler, inputFD, tokensNumber, status);
	//pkglist_print (handler);
	//initializeClient (handler, 0, kPackages);
	//return temporaneo
	//return 0;
	int sockfd , client_sock , clientsize;
	struct sockaddr_in *server , client;
	//porta tcp random nel range assegnato da IANA per l'utente
	srand (time(NULL));
	int port = randomIntGenerator (1024, 1030);
	//int port = randomIntGenerator (1024, 49150);


	//maxOperatorsQueue indica quanti operatori al massimo
	//possno essere accodati sulla listen prima di essere
	//accettati.
	int maxOperatorsQueue = 128;
	sockfd = InitSocket (server, port, maxOperatorsQueue);

	connectionManager (sockfd, operatorsNumber, client, clientsize);

	close (sockfd);
	pthread_mutex_destroy(&maxThreadsMutex);
	perror ("fine programma");
	return 0;
}

void connectionManager (int sockfd, int operatorsNumber, struct sockaddr_in client, int clientsize) {
	
	char 			tids[32];
	int 			client_sock;
	pthread_t 		thread_id;
	int 			tid;
	
	tid 			= 0;
	maxThread 		= 0;
	clientsize 		= sizeof(struct sockaddr_in);
	char *mess00	= "Connessione accettata\n";
	char *mess01	= "Thread assegnato\n";

	//potrei aver bisogno di un array di sock ed un array di client.

	while (1) {

		if (maxThread < operatorsNumber ) {
			client_sock = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&clientsize);
			if ( client_sock != -1) {
				write (STDOUT_FILENO, mess00, strlen (mess00));

				if( pthread_create (&thread_id , NULL ,  connection_handler , (void*) &client_sock) == 0) {
					sprintf  (tids, "nuovo tid: [%d] ", tid);
					tid++;

					if ( (pthread_mutex_lock (&maxThreadsMutex)) == 0) {
						maxThread++;
						pthread_mutex_unlock (&maxThreadsMutex);
					} else
						perror ("server-mutex:errore");

					write (STDOUT_FILENO, tids, strlen (tids));	 
					write (STDOUT_FILENO, mess01, strlen (mess01));
				
					//pthread_join (thread_id , NULL);
				} else 
					perror ("impossibile creare il thread");
			} else {
				perror ("impossibile accettare la connessione\n");
			}

		}

		int err = pthread_mutex_lock (&maxThreadsMutex);

		sprintf (tids, "threads correnti: [%d] \n", maxThread);
		err = pthread_mutex_unlock (&maxThreadsMutex);
		
		write (STDOUT_FILENO, tids, strlen (tids));	
		memset (tids, '\0', strlen (tids));
	
		
	}


	//pthread_join (thread_id , NULL);
	while (1) {
		sleep (1);
	}	
}

/*
 * thread per client
 * */
void *connection_handlerA(void *sockfd) {
	//alloca il socket descriptor
	int sock = *(int*)sockfd;
	int read_size;
	char *message , client_message[2000];

	//messaggio di prova
	message = "connection handler\n";
	write(sock , message , strlen(message));

	message = "messaggio:\n";
	write(sock , message , strlen(message));

	//ricezione messaggio
	while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 ) {
		//marker fine stringa
		client_message[read_size] = '\0';

		//reinvia il messaggio al client
		write(sock , client_message , strlen(client_message));

		//azzera la memoria buffer
		memset(client_message, 0, 2000);
	}

	if(read_size == 0) {
		write(STDOUT_FILENO, "Client disconnesso\n", sizeof ("Client disconnesso\n"));
		fflush(stdout);
	}
	else if(read_size == -1) {
		perror("ricezione fallita");
	}

	return 0;
}
