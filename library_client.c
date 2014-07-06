#include "library.h"

void commandSwitch (int command, char *cmdPointer, Package *handler, int sockfd) {
	char *err01 = "warning! comando non valido!\n";

	switch (command) {

		case ELENCASERVER:
		/*ELENCA STAMPA LA LISTA REMOTA*/
			write (STDOUT_FILENO, cmdPointer, strlen (cmdPointer));
			//elencaserver_client (sockfd, cmdPointer);
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
			pkglist_print (handler);
		break;						
		default:
			write (STDOUT_FILENO, err01, strlen (err01));
		break;
	}
}


void elencaserver_client (int sockfd, char *cmdPointer) {

	int i;
	int check = 1;
	char *ptr;
	char c[5];
	write (sockfd, cmdPointer, strlen (cmdPointer));
	char *strbuffer = (char *) malloc (256 * sizeof (char));	
	memset (strbuffer, 0, strlen (strbuffer));
	while (check != 0 && (readLine (sockfd, strbuffer)) > 0) {
		//funzione di libreria che cerca una sottostringa
		//in una sottostringa e restituisce il puntatore
		ptr = strstr (strbuffer, "EOM#");
		if (ptr == NULL) {
			write (STDOUT_FILENO, strbuffer, strlen (strbuffer));
			write (STDOUT_FILENO, "\n", 1);
		} else
			check = 0;
		memset (strbuffer, 0, strlen (strbuffer));	
	}

	free (strbuffer);

}
