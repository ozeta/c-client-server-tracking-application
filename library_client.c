#include "library.h"


int initClientSocket (char **argv) {

	char *address;
	struct addrinfo hints, *res;
	memset (&hints, 0, sizeof (hints));
	char *mess00 = "ip agganciato...\n";
	char *mess01 = "socket creato...\n";
	char *mess02 = "connessione eseguita.\n\n";
	char *mess03 = "impossibile connettersi.\n";
	hints.ai_family		= AF_INET; //protocollo ipv4
	hints.ai_socktype   = SOCK_STREAM; //tcp
	hints.ai_flags		= AI_PASSIVE; //INADDR_ANY
	int sockfd;
		//argv[1] = ip; argv[2] = port
	if (getaddrinfo (argv[1], argv[2], &hints, &res) == -1)
		perror ("Error1: "), exit (-1);
	write (STDOUT_FILENO, mess00, strlen (mess00));
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd == -1)
		perror ("Error2: "), exit (-1);
	write (STDOUT_FILENO, mess01, strlen (mess01));

	int bindErr;
	int timeout = 12;
	while ((connect (sockfd, res->ai_addr, res->ai_addrlen) < 0) && timeout > 0) {
		perror ("connect >  Nuovo tentativo di connessione tra 5 secondi"), sleep (5);
		timeout++;
	}
	//se il timeout scade il programma chiude
	if (timeout == 0)
		write (STDERR_FILENO, mess03, strlen (mess03)), exit (-1);
	
	write (STDOUT_FILENO, mess02, strlen (mess02));
	return sockfd;
}


Package *commandSwitch (int command, char *strbuffer, Package *handler, int sockfd) {
	char *err01 = "warning! comando non valido!\n";

	switch (command) {

		case ELENCASERVER:
		/*STAMPA LA LISTA REMOTA*/
			//write (STDOUT_FILENO, strbuffer, strlen (strbuffer));
			elencaserver_client (sockfd, strbuffer);
		break;
		case CONSEGNATO:
			//write (STDOUT_FILENO, "switch-> consegnato\n", strlen ("switch-> consegnato\n"));
			handler = consegnato_client (sockfd, strbuffer, handler);
			//cercapacchetto
			//inviapacchetto
			//cancellapacchetto
		break;
		case RITIRATO:
			//write (STDOUT_FILENO, "switch-> ritirato\n", strlen ("switch-> ritirato\n"));
			ritirato_client (sockfd, strbuffer, handler);
		break;
		case SMISTA:
			//write (STDOUT_FILENO, "switch-> smista\n", strlen ("switch-> smista\n"));
			handler = smista_client (sockfd, strbuffer, handler);
		break;
		case ELENCA:
		/*ELENCA STAMPA LA LISTA LOCALE*/
			//write (STDOUT_FILENO, "switch-> elenca:\n", strlen ("switch-> elenca:\n"));
			pkglist_print (handler);
		break;						
		default:
			write (STDOUT_FILENO, err01, strlen (err01));
		break;
	}
	return handler;
}


void elencaserver_client (int sockfd, char *cmdPointer) {

	int i;
	int check = 1;
	char *ptr;
	char c[5];
	char *strbuffer = (char *) malloc (256 * sizeof (char));	
	memset (strbuffer, 0, strlen (strbuffer));
	write (sockfd, cmdPointer, strlen (cmdPointer));
	//write (sockfd, "\n", 1);
	
	while (check != 0 && (readLine (sockfd, strbuffer)) > 0) {
		//funzione di libreria che cerca una sottostringa
		//in una sottostringa e restituisce il puntatore
		ptr = strstr (strbuffer, "EOM#");
		if (ptr == NULL) {
			strbuffer = decodePkgfromTransmission (strbuffer);
			write (STDOUT_FILENO, strbuffer, strlen (strbuffer));
			//write (STDOUT_FILENO, "\n", 1);
		} else
			check = 0;
		memset (strbuffer, 0, strlen (strbuffer));	
	}

	free (strbuffer);
}


int checkCommandInput (char *strbuffer, int parameters) {

	int i = 0;
	int end = strlen (strbuffer);
	int res = 0;
	while (i < end) {
		if (strbuffer[i] == '#')
			res++;	
		i++;
	}

	if (parameters == res)
		return 1;
	else
		return 0;
}

void ritirato_clientA (int sockfd, char *strbuffer, Package *handler) {
	//array semidinamico: riceve l'input dalla funzione
	//chiamante
	int check;
	if ((check = checkCommandInput (strbuffer, 3)) != 0) {

		int tokensNumber = 3;
		char *str[tokensNumber];
		int lenght = strlen (strbuffer);
		strbuffer[lenght-1] = '\0';
		int i;
		for (i = 0; i < tokensNumber; i++) {
			str[i] = (char *) malloc (256 * sizeof (char));
			memset (str[i], '\0', strlen (str[i]));
		}
		Status status = COLLECTED;
		getTokens (str, &strbuffer[9], tokensNumber);
		handler = pkg_enqueue (handler, str, status);
		strbuffer[lenght-1] = '\n';		
		if ((write (sockfd, strbuffer, strlen (strbuffer))) == -1) {
			perror ("impossibile inviare il messaggio: operazione abortita");
		}
		for (i = 0; i < tokensNumber; i++) { //libero i puntatori
			free (str[i]);
		}
	} else {
		char warn1[] = "attenzione, comando non corretto\n";
		char warn2[] = "esempio: ritirato#codice#descrizione#indirizzo\n";
		write (STDOUT_FILENO, warn1, strlen (warn1));
		write (STDOUT_FILENO, warn2, strlen (warn2));
	}

}

Package *consegnato_client (int sockfd, char *strbuffer, Package *handler) {

		//consegnato#codice
		//cercapacchetto
		//inviapacchetto
		//cancellapacchetto
	int check;
	if ((check = checkCommandInput (strbuffer, 1)) != 0) {

		int tokensNumber = 2;
		char *str[tokensNumber];
		int lenght = strlen (strbuffer);
		strbuffer[lenght-1] = '\0';
		int i;
		for (i = 0; i < tokensNumber; i++) {
			str[i] = (char *) malloc (256 * sizeof (char));
			memset (str[i], '\0', strlen (str[i]));
		}
		getTokens (str, strbuffer, tokensNumber);

		Package *result = pkg_find (handler,str[1]);
		if (result != NULL && result->stato_articolo == TOBEDELIVERED) {
			strbuffer[lenght-1] = '\n';		
			if ((write (sockfd, strbuffer, strlen (strbuffer))) != -1) {
				handler = pkg_delete (handler, str[1]);							
			} else {
				perror ("impossibile inviare il messaggio. operazione abortita");
			}
		} else {
			char warn0[] = "attenzione: pacchetto non valido o non esistente\n";
			write (STDOUT_FILENO, warn0, strlen (warn0));
		}
		for (i = 0; i < tokensNumber; i++) { //libero i puntatori
			free (str[i]);
		}
	} else {
		char warn1[] = "attenzione: comando non corretto\n";
		char warn2[] = "esempio: consegnato#codice\n";
		write (STDOUT_FILENO, warn1, strlen (warn1));
		write (STDOUT_FILENO, warn2, strlen (warn2));
	}

	return handler;
}

Package *smista_client (int sockfd, char *strbuffer, Package *handler) {
	int check;
	if ((check = checkCommandInput (strbuffer, 1)) != 0) {

		int tokensNumber = 2;
		char *str[tokensNumber];
		int lenght = strlen (strbuffer);
		strbuffer[lenght-1] = '\0';
		int i;
		for (i = 0; i < tokensNumber; i++) {
			str[i] = (char *) malloc (256 * sizeof (char));
			memset (str[i], '\0', strlen (str[i]));
		}
		getTokens (str, strbuffer, tokensNumber);

		Package *result = pkg_find (handler,str[1]);

		if (result != NULL && result->stato_articolo == COLLECTED) {
			strbuffer[lenght-1] = '\n';		
			if ((write (sockfd, strbuffer, strlen (strbuffer))) != -1) {
				handler = pkg_delete (handler, str[1]);							
			} else {
				perror ("impossibile inviare il messaggio. operazione abortita");
			}
		} else {
			char warn0[] = "attenzione: pacchetto non valido o non esistente\n";
			write (STDOUT_FILENO, warn0, strlen (warn0));
		}
		for (i = 0; i < tokensNumber; i++) { //libero i puntatori
			free (str[i]);
		}
	} else {
		char warn1[] = "attenzione: comando non corretto\n";
		char warn2[] = "esempio: smista#codice\n";
		write (STDOUT_FILENO, warn1, strlen (warn1));
		write (STDOUT_FILENO, warn2, strlen (warn2));
	}

	return handler;
}


void ritirato_clientAA (int sockfd, char *strbuffer, Package *handler) {
	//array semidinamico: riceve l'input dalla funzione
	//chiamante
	int check;
	char messOk[] = "inserimento avvenuto con successo\n";
	char messNo[] = "inserimento non concesso: codice pacchetto gia' esistente\n";

	if ((check = checkCommandInput (strbuffer, 3)) != 0) {

		int tokensNumber = 3;
		char *str[tokensNumber];
		if ((write (sockfd, strbuffer, strlen (strbuffer))) == -1) {
			perror ("impossibile inviare il messaggio: operazione abortita");
		}
		char result[10];
		memset (result, 0, sizeof(result));
		read (sockfd, result, 2);
		perror (result);
		if ((strcmp (result, "OK")) == 0) {
			write (STDOUT_FILENO, "--ok--\n\n", 8);
			int lenght = strlen (strbuffer);
			strbuffer[lenght-1] = '\0';
			int i;
			for (i = 0; i < tokensNumber; i++) {
				str[i] = (char *) malloc (256 * sizeof (char));
				memset (str[i], '\0', strlen (str[i]));
			}
			Status status = COLLECTED;
			getTokens (str, &strbuffer[9], tokensNumber);
			handler = pkg_enqueue (handler, str, status);
			for (i = 0; i < tokensNumber; i++) { //libero i puntatori
				free (str[i]);
			}
		} else {
			write (STDOUT_FILENO, messNo, strlen (messNo));

		}

	} else {
		char warn1[] = "attenzione, comando non corretto\n";
		char warn2[] = "esempio: ritirato#codice#descrizione#indirizzo\n";
		write (STDOUT_FILENO, warn1, strlen (warn1));
		write (STDOUT_FILENO, warn2, strlen (warn2));
	}

}


void ritirato_client (int sockfd, char *strbuffer, Package *handler) {
	//array semidinamico: riceve l'input dalla funzione
	//chiamante
	int check;
	char result[32];
	memset (result, 0, sizeof(result));
	if ((check = checkCommandInput (strbuffer, 3)) != 0) {

		int tokensNumber = 3;
		char *str[tokensNumber];
		write (sockfd, strbuffer, strlen (strbuffer));
		readLine (sockfd, result);
		char ok[] = "INSOK";
		char no[] = "NOTOK";
		//write (STDOUT_FILENO, strbuffer, strlen (strbuffer));				

		if ((strcmp (result, ok)) == 0) {
			write (STDOUT_FILENO, "--ok--\n\n", 8);

		} else {
			write (STDOUT_FILENO, "--no--\n\n", 8);
		}
			
	} else {
		char warn1[] = "attenzione, comando non corretto\n";
		char warn2[] = "esempio: ritirato#codice#descrizione#indirizzo\n";
		write (STDOUT_FILENO, warn1, strlen (warn1));
		write (STDOUT_FILENO, warn2, strlen (warn2));
	}

}
