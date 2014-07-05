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
	//signal(SIGINT,SIG_IGN);
	int inputFD = serverInputCheck(argc, argv);
	int opNumber = atoi (argv[1]);
	int kPackages		= atoi (argv[2]);
	int err;
	int tokensNumber	= 3;	
	int sockfd , client_sock , clientsize;
	int maxOperatorsQueue = 128;
	Package *handler	= NULL;
	Status status = STORAGE;

	if ((err = pthread_mutex_init (&maxThreadsMutex, NULL)) != 0) 
		perror ("Impossibile allocare il mutex per i thread"), exit (-1);
	if ((err = pthread_mutex_init (&packageMutex, NULL)) != 0) 
		perror ("Impossibile allocare il mutex per la lista"), exit (-1);

	handler = (Package *) createList (handler, inputFD, tokensNumber, status);
	//pkglist_print (handler);
	//return 0;
	struct sockaddr_in *server , client;
	//porta tcp random nel range assegnato da IANA per l'utente
	srand (time(NULL));
	int port = randomIntGenerator (1024, 1030);
	//int port = randomIntGenerator (1024, 49150);


	//maxOperatorsQueue indica quanti operatori al massimo
	//possno essere accodati sulla listen prima di essere
	//accettati.
	sockfd = InitSocket (server, port, maxOperatorsQueue);

	connectionManager (sockfd, opNumber, kPackages, handler, client, clientsize);

	close (sockfd);
	pthread_mutex_destroy(&maxThreadsMutex);
	perror ("fine programma");
	return 0;
}

void connectionManager (int sockfd, int opNumber, int kPackages, Package *handler,
                        struct sockaddr_in client, int clientsize) {
	
	char 			tids[32];
	int 			client_sock;	
	int 			tid;
	int 			err;
	pthread_t 		thread_id;
	pthread_t 		*thread_arr = (pthread_t *) malloc (opNumber * sizeof (pthread_t));	
	pthread_cond_t  condition;
	tid 			= 0;
	maxThread 		= 0;
	clientsize 		= sizeof(struct sockaddr_in);
	char *mess00	= "Connessione accettata\n";
	char *mess01	= "Thread assegnato\n";

	while (1) {

		if (maxThread < opNumber ) {
			client_sock = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&clientsize);
			if ( client_sock != -1) {
				write (STDOUT_FILENO, mess00, strlen (mess00));
				Passaggio param_pass;
				memset (&param_pass, 0, sizeof (Passaggio));
				param_pass.sockfd		= client_sock;
				param_pass.kPackages 	= kPackages;
				param_pass.handler 		= handler;
				//printf ("--%d---\n", param_pass.sockfd);
				if( pthread_create (&thread_arr[maxThread] ,
				   					NULL ,
				   					connection_handler ,
				   					(void *) &param_pass) == 0)

				{
					sprintf  (tids, "nuovo tid: [%d] ", tid);
					tid++;

					if ( (pthread_mutex_lock (&maxThreadsMutex)) == 0) {
						maxThread++;
						pthread_mutex_unlock (&maxThreadsMutex);
					} else
						perror ("server-mutex: errore");

					//write (STDOUT_FILENO, tids, strlen (tids));	 
					//write (STDOUT_FILENO, mess01, strlen (mess01));
				
					//pthread_join (thread_arr[maxThread] , NULL);
				} else 
					perror ("impossibile creare il thread");
			//
			} else {
				perror ("impossibile accettare la connessione");
			}

		}
		/*
		int err = pthread_mutex_lock (&maxThreadsMutex);

		sprintf (tids, "thread correnti: [%d] \n", maxThread);
		err = pthread_mutex_unlock (&maxThreadsMutex);
		
		write (STDOUT_FILENO, tids, strlen (tids));	
		memset (tids, '\0', strlen (tids));
		sleep (1);
		*/
		//free (param_pass);
		
	}

}
