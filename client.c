#include "library.h"

char *cliCommands[5] = {
	"ELENCASERVER",
	"CONSEGNATO",
	"RITIRATO",
	"SMISTA",
	"ELENCA"
};

Package *initClient (int sockfd, Package *handler) {
	//pkg handler, socket, tokens, status
	//initClient si mette in attesa di ricevere pacchetti, anche
	//se il server ne è momentaneamene sprovvisto
	while ((handler = createList (handler, sockfd,  4, -1)) == NULL) {
		usleep (50000);
	}

	pkglist_print (handler);

	return handler;
}

int main(int argc, char **argv) {

	char mess[] 	= "inizializzazione terminata.\n";
	char mess1[] 	= "comando-> ";
	clientInputCheck(argc, argv);
	int sockfd = initClientSocket (argv);
	Package *handler = NULL;
	handler = initClient (sockfd, handler);
	write (STDOUT_FILENO, mess, strlen (mess));
	while (handler != NULL) {
		write (STDOUT_FILENO, mess1, strlen (mess1));	
		showMenu();
		int command = getLine (STDIN_FILENO, cliCommands);
		commandSwitch (command, handler, sockfd);
	}
	write (STDOUT_FILENO, "Ciao!\n", 6);
	close (sockfd);

	return 0;
}

