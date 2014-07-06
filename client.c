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
	handler = createListA  (handler, sockfd,  4, -1);
	pkglist_print (handler);

	return handler;
}

int main(int argc, char **argv) {

	clientInputCheck(argc, argv);
	int sockfd = initClientSocket (argv);
	Package *handler = NULL;
	handler = initClient (sockfd, handler);

	write (STDOUT_FILENO, "ok\n", 3);
	close (sockfd);

	return 0;
}



	//leggo da stdin
	//estrapolo il primo token
	//lo comparo con l'array client
	//estraggo l'indice dell'array
	//switcho sui 5 casi possibili
/*
int command = getLine (STDIN_FILENO, cliCommands);

int inputFD = open ("test.txt", O_RDONLY);
int tokensNumber = 3;
Status status = DELIVERED;
handler = (Package *) createList (handler, inputFD, tokensNumber, status);
commandSwitch (command, handler);
*/
/*
void getLineA (int inputFD) {
	char *strbuffer = malloc (256);
	memset (strbuffer, 0, strlen (strbuffer));
	int inputFile = 0;
	int rVar = getLine (inputFile, strbuffer);
}
*/
/*
void initializeClient (Package *handler, int clientSock, int kPackages) {

	int i = 0;
	Package *temp = NULL;
	Status stat = STORAGE;
	while (i < kPackages) {
		temp = getStoredPackage (handler, DELIVERED);
		temp->stato_articolo = TOBEDELIVERED;
	//	if (temp != NULL)
	//		pkg_print (temp);
	}

}

*/
