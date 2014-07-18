/*=============================================================================
  Nome: server.c
  Autori:
	MARCO CARROZZO     	N86/1240
	MAURIZIO DEL PRETE 	N86/783

	Progetto: Corriere Espresso
  ===========================================================================*/
/*
	compilazione:
	gcc client.c library.c library_client.c -o client
*/

#include "library_client.h"



int main( int argc, char **argv ) {

	char 		mess[] 			= "\nInizializzazione terminata.\n";
	char 		mess1[] 		= "comando-> ";
	int 		sockfd;
	Package *	handler;
	clientInputCheck( argc, argv );													//controlla i parametri dati in input
	sockfd 						= initClientSocket ( argv );						//inizializza socket e connessione
	handler			 			= NULL;
	handler 					= initClient ( sockfd, handler );					//inizializza il client. riceve i k pacchetti da consegnare
	write ( STDOUT_FILENO, mess, strlen ( mess ) );
	showMenu();																		//mostra il menu dei comandi
	while ( handler != NULL ) {														//il client vive finche' la lista non e' nulla
		char *strbuffer;
		write ( STDOUT_FILENO, mess1, strlen ( mess1 ) );	
		int command = getCommand ( STDIN_FILENO, &strbuffer );						//legge un comando da standard input
		handler 	= commandSwitch ( command, strbuffer, handler, sockfd );		//interpreta il comando ed esegue l'azione corrispondente
		free ( strbuffer );															//dealloca strbuffer
	}
	sendMessage ( sockfd, "esci\n");
	write ( STDOUT_FILENO, "Ciao!\n", 6 );
	close ( sockfd );																//chiude il socket

	return 0;
}
