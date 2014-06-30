	
	/*====================================
    int port = -1;
	
    //creo il socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1)
    {
        printf("Non è possibile creare il socket");
    }
    puts("Socket creato");
     
	//ma il server che indirizzo ip deve avere?!
    //preaparo la struttura sockaddr_in che contenga le informazioni di connessione
	server = (struct sockaddr_in *) malloc (sizeof (struct sockaddr_in));
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = INADDR_ANY;
    server->sin_port = htons( 8888 );
     
    //effettuo la Bind e gestisco l'eventuale errore

	//processo che tenta la bind finché non si libera la porta
    int bindErr;
	do {
        bindErr = bind(sockfd,(struct sockaddr *)server , sizeof(struct sockaddr_in));
		int errsv = errno;
		if (bindErr < 0) {
			if ( errsv == EINVAL || errsv == EADDRINUSE )
				perror ("Bind > Error3: Nuovo tentativo di assegnazione tra 5 secondi"), sleep (5);
			else
				perror ("Bind > Error4: exiting...."), exit (-1);
		} else if (bindErr == 0)
			write (STDOUT_FILENO, "Bind eseguita.\n", sizeof ("Bind eseguita.\n"));  
    } while (bindErr < 0);




	
	//togli sto puts
    puts("bind done");
     
    //avvio la Listen su un numero N di operatori
    listen(sockfd , operatorsNumber);
	
	//togli sto puts    
    puts("Waiting for incoming connections...");
    
	====================================*/
	/*
	while (err > 0 && (buffer[0] != '#' || buffer[0] != '\n' || buffer[0] != '\0')) {
	err = read (inputFile, buffer, 1);

	write (STDOUT_FILENO, &buffer[0], err);
	//write (STDOUT_FILENO, " ", 1);
} ;


	while (err > 0) {
		//for (i = 0; i < 3; i++) {
			do {
				err = read (inputFile, buffer, 1);

				write (STDOUT_FILENO, &buffer[0], err);
				//write (STDOUT_FILENO, " ", 1);
			} while (err > 0 && (buffer[0] != '#' || buffer[0] != '\n' || buffer[0] != '\0'));
			//}
	} 
	
	
	while file non è terminato
	ripeti 3 volte
		leggi la stringa fino al # o \n o \0 e salvala in buffer[0]
		copia la stringa buffer in string[i].

			
	while ( err > 0) {
	int i;
		for (i = 0; i < 3; i++) {
			do {
				err = read (inputFile, buffer, 1);
				write (STDOUT_FILENO, &buffer[0], err);
			} while ((buffer[0] != '#' || buffer[0] != '\n' || buffer[0] != '\0'));
		}
	}


		if (buffer[0] == '\n')
			buffer[0] = '\n';
		write (STDOUT_FILENO, buffer, 1);

/*holy shit it works! 30 giugno*/
/*
	int createList(Package *handler, int inputFile) {
	char *string[3];
	int i = 0;
	for (; i < 3; i++) {
		string[i] = (char *) malloc (256);
		memset (string[i], '\0', strlen (string[i]));
	}
	char buffer[2];
	char *strbuffer = malloc (256);
	memset (buffer, '\0', 2);	
	memset (strbuffer, 0, strlen (strbuffer));
	char *token;

	int rVar = 1;
	int wVar;
	i = 0;

	while ( (rVar = getLineFromFile (inputFile, strbuffer))> 0 ) {

			write (STDOUT_FILENO, strbuffer, strlen (strbuffer));
			write (STDOUT_FILENO, "\n", 1);

	}
	write (STDOUT_FILENO, "\n", 1);
	close (inputFile);
	
	return 0;
}
	*/