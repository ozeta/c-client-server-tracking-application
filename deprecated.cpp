//client.c -> while

	while (handler != NULL) {
		write (STDOUT_FILENO, mess1, strlen (mess1));	

		showMenu();
		char *strbuffer;
		strbuffer = (char *) malloc (256 * sizeof (char));	
		memset (strbuffer, 0, 256 * sizeof (char));
		write (STDOUT_FILENO, "test: ", sizeof ("test: "));		
		write (STDOUT_FILENO, test, strlen (test));
		write (STDOUT_FILENO, "\n", 1);	
		//commandSwitch (command, strbuffer, handler, sockfd);
		free (strbuffer);
	}

	int getLineB (int inputFD, char *strbuffer) {

	int rVar = 1;
	int command;
	char *string = (char *) malloc (256 * sizeof (char));
	memset (string, '\0', strlen (string));	
	if ( (rVar = readLine (inputFD, strbuffer)) > 0 ) {

		getCommand (string, strbuffer);
		command = commandToHash (string);


	}
	return command;
}

int getLineA (int inputFD, char *strbuffer, char **test1) {
	int **ipp;
	int *ip1;
	int i = 1;
	ip1 = &i;
	*ipp = ip1;
	*ipp = &i;
/**/

	int rVar = 1;
	int command;
	char *test;
	char *string = (char *) malloc (256 * sizeof (char));
	memset (string, '\0', strlen (string));	
	test =  (char *) malloc (256 * sizeof (char));
	memset (test, '\0', strlen (test));	
	if ( (rVar = readLine (inputFD, strbuffer)) > 0 ) {

		getCommand (string, strbuffer);
		command = commandToHash (string);
		*test1 = test;
		strcpy (test, strbuffer);

	}
	return command;
}


Package * createList (Package *handler, int inputFD, int tokensNumber, int status) {
	//array semidinamico: riceve l'input dalla funzione
	//chiamante
	char *str[tokensNumber];
	int i;
	int check = 1;
	char *ptr;
	for (i = 0; i < tokensNumber; i++) {
		str[i] = (char *) malloc (256 * sizeof (char));
		if (str[i] == NULL)
			perror ("memoria esaurita"), exit (-1);
		memset (str[i], '\0', strlen (str[i]));
	}
	char *strbuffer = malloc (256 * sizeof (char));
	if (strbuffer == NULL)
		perror ("memoria esaurita"), exit (-1);	
	memset (strbuffer, 0, strlen (strbuffer));
	while (check != 0 && (readLine (inputFD, strbuffer)) > 0) {
		//funzione di libreria che cerca una sottostringa
		//in una sottostringa e restituisce il puntatore
		ptr = strstr (strbuffer, "EOM#");
		if (ptr == NULL) {
			getTokens (str, strbuffer, tokensNumber);
			/**implementazione con push su "pila"*/
			handler = pkg_push (handler, str, status);
			pkg_print (handler);
			for (i = 0; i < tokensNumber; i++)
				memset (str[i], '\0', strlen (str[i]));
			memset (strbuffer, 0, strlen (strbuffer));
		} else
			check = 0;	
	}

	for (i = 0; i < tokensNumber; i++) {
			free (str[i]);
	}
	free (strbuffer);
	
	return handler;
}