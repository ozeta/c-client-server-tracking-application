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
	int command = getLine (STDIN_FILENO, cliCommands);
	Package *handler = NULL;
	
	int test = open ("test.txt", O_RDONLY);
	handler = (Package *)createList (handler, test, 3);
	commandSwitch (command, handler);

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
