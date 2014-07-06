	while ( (readLine (inputFD, strbuffer)) > 0 && check != 0) {
		
		getTokens (str, strbuffer, tokensNumber);
		check = isEndOfMessage(str[0]);
		if ( check != 0) {
			/**implementazione con push su "pila"*/
			handler = pkg_push (handler, str, status);
			for (i = 0; i < tokensNumber; i++)
				memset (str[i], '\0', strlen (str[i]));
		}
	}
			ptr = strstr ( strbuffer, "EOM#" );
		if (ptr != NULL) {
			printf ("ok");
		}


Package * createList(Package *handler, int inputFD, int tokensNumber, int status) {
	char *str[tokensNumber];
	int i;
	int check = 1;
	for (i = 0; i < tokensNumber; i++) {
		str[i] = (char *) malloc (256);
		memset (str[i], '\0', strlen (str[i]));
	}

	char *strbuffer = malloc (256);	
	memset (strbuffer, 0, strlen (strbuffer));

	while ( (readLine (inputFD, strbuffer)) > 0 && check != 0) {
		
		getTokens (str, strbuffer, tokensNumber);
		//check = isEndOfMessage(str[0]);
		if ( check != 0) {
			/**implementazione con push su "pila"*/
			handler = pkg_push (handler, str, status);
			for (i = 0; i < tokensNumber; i++)
				memset (str[i], '\0', strlen (str[i]));
		}
	}
	//write (STDOUT_FILENO, "\n", 1);


	for (i = 0; i < tokensNumber; i++)
		free (str[i]);
	free (strbuffer);
	
	return handler;
}

Package * createListA (Package *handler, int inputFD, int tokensNumber, int status) {
	char *str[tokensNumber];
	int i;
	int check = 1;
	char *ptr;
	for (i = 0; i < tokensNumber; i++) {
		str[i] = (char *) malloc (256);
		memset (str[i], '\0', strlen (str[i]));
	}
	char *strbuffer = malloc (256);	
	memset (strbuffer, 0, strlen (strbuffer));
	while ((readLine (inputFD, strbuffer)) > 0 check != 0) {
		ptr = strstr(strbuffer, "EOM#");
		if (ptr == NULL) {
			write (STDOUT_FILENO, strbuffer, strlen (strbuffer));
			write (STDOUT_FILENO, "\n", 1);
			memset (strbuffer, 0, strlen (strbuffer));
		} else
			check = 0;	
	}

	for (i = 0; i < tokensNumber; i++)
		free (str[i]);
	free (strbuffer);
	
	return handler;
}