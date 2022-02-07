
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "interpreter.h"
#include "shellmemory.h"


int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);
int preprocess(char ui[]);

// Start of everything
int main(int argc, char *argv[]) {

	printf("%s\n", "Shell version 1.1 Created January 2022");
	help();

	char prompt = '$';  				// Shell prompt
	char userInput[MAX_USER_INPUT];		// user's input stored here
	int errorCode = 0;					// zero means no error, default

	//init user input
	for (int i=0; i<MAX_USER_INPUT; i++)
		userInput[i] = '\0';
	
	//init shell memory
	mem_init();

	while(1) {							
		if (feof(stdin)) { // if end of file reached, switch input back to keyboard
			freopen("/dev/tty","rw",stdin);
		}
		printf("%c ",prompt);
		fgets(userInput, MAX_USER_INPUT-1, stdin);

		errorCode = preprocess(userInput);	// separate and run same line commands
		if (errorCode == -1) exit(99);	// ignore all other errors

		memset(userInput, 0, sizeof(userInput));

	}

	return 0;

}

// Extract words from the input then call interpreter
int parseInput(char ui[]) {
 
	char tmp[200];
	char *words[100];							
	int a,b;							
	int w=0; // wordID

	for(a=0; ui[a]==' ' && a<1000; a++);		// skip white spaces

	while(ui[a] != '\0' && a<1000) {

		for(b=0; ui[a]!='\0' && ui[a]!=' ' && a<1000; a++, b++)
			tmp[b] = ui[a];						// extract a word
	 
		tmp[b] = '\0';

		words[w] = strdup(tmp);

		a++; 
		w++;
	}

	return interpreter(words, w);
}

// Separate input for multiple commands on the same line, and feed them to parseInput
int preprocess(char ui[]) {
	char tmp[200];
	char** words = malloc(10 * sizeof(char*));
	int a,b;							
	int w=0; // wordID
	int errorCode = 0;					// zero means no error, default

	for(a=0; ui[a]==' ' && a<1000; a++);		// skip white spaces

	while(ui[a] != '\0' && a<1000) {

		for(b=0; ui[a]!='\0' && ui[a]!=';' && a<1000; a++, b++) // Match end of line or semi-colon
			tmp[b] = ui[a];						// extract a command
	 
		tmp[b] = '\0';

		words[w] = strdup(tmp);
		errorCode = parseInput(words[w]);		// run commands one by one
		if (errorCode == -1) exit(99);	// ignore all other errors

		a++; 
		w++;
	}

	return 0;
}
