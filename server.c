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
void *connection_handler(void *);

int main(int argc , char *argv[])
{
	//check preliminare su input
	int inputFD = serverInputCheck(argc, argv);
	int operatorsNumber = atoi (argv[1]);
	int kPackages		= atoi (argv[2]);
	Package *handler	= NULL;
	//scrivi funzione createList
    int tokensNumber = 3;
	handler = (Package *)createList (handler, inputFD, tokensNumber);
    pkglist_print (handler);
    //initializeClient (handler, 0, kPackages);
    //return temporaneo
	return 0;
    int sockfd , client_sock , clientsize;
    struct sockaddr_in *server , client;
	//porta tcp random nel range assegnato da IANA per l'utente
    srand (time(NULL));
	int port = randomIntGenerator (1024, 49150);
	sockfd = InitSocket (server,port,operatorsNumber);

	//
	clientsize = sizeof(struct sockaddr_in);
	pthread_t thread_id;

	//potrei aver bisogno di un array di client_sock ed un array di client.
    while( (client_sock = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&clientsize)) )
    {
		//togli sto puts
        puts("Connection accepted");

        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //join dei thread, per evitare che il master thread chiuda prima del termine dei figli.
        pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}

/*
 * thread per client
 * */
void *connection_handler(void *sockfd)
{
    //alloca il socket descriptor
    int sock = *(int*)sockfd;
    int read_size;
    char *message , client_message[2000];

    //messaggio di prova
    message = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));

    message = "Now type something and i shall repeat what you type \n";
    write(sock , message , strlen(message));

    //ricezione messaggio
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //marker fine stringa
		client_message[read_size] = '\0';

		//reinvia il messaggio al client
        write(sock , client_message , strlen(client_message));

		//azzera la memoria buffer
		memset(client_message, 0, 2000);
    }

    if(read_size == 0)
    {
        puts("Client disconnesso");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("ricezione fallita");
    }

    return 0;
}
