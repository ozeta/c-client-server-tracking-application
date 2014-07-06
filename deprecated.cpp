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