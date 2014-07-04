#include "library.h"

char *cliCommands[5] = {
	"ELENCASERVER",
	"CONSEGNATO",
	"RITIRATO",
	"SMISTA",
	"ELENCA"
};

int main(int argc, char **argv) {

	clientInputCheck(argc, argv);
	int sockfd = connectToServer (argv);
	char buff[5];
	memset (buff, '\0', strlen(buff));
	int rVar;
	while ((rVar = read (sockfd, buff, 1)) != -1) {

		write (STDOUT_FILENO, buff, 1);
		memset (buff, '\0', strlen(buff)); 

	}
	/*
	int command = getLine (STDIN_FILENO, cliCommands);
	Package *handler = NULL;
	
	int inputFD = open ("test.txt", O_RDONLY);
	int tokensNumber = 3;
	Status status = DELIVERED;
	handler = (Package *) createList (handler, inputFD, tokensNumber, status);
	commandSwitch (command, handler);
	*/
	close (sockfd);
	//leggo da stdin
	//estrapolo il primo token
	//lo comparo con l'array client
	//estraggo l'indice dell'array
	//switcho sui 5 casi possibili
	return 0;
}

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
