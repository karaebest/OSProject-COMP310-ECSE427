#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 7; //changed to accomodate 5 tokens in set

int help();
int quit();
int badcommand();
int set(char* var, char* value);
int print(char* var);
int run(char* script);
int my_ls();
int badcommandFileDoesNotExist();
int badcommandTooManyTokens();
int echo(char* value);

// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size){
	int i;

	if ( args_size < 1 || (args_size > MAX_ARGS_SIZE && strcmp(command_args[0], "set")!=0)){
		return badcommand();
	}


	for ( i=0; i<args_size; i++){ //strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "help")==0){
	    //help
	    if (args_size != 1) return badcommand();
	    return help();
	
	} else if (strcmp(command_args[0], "quit")==0) {
		//quit
		if (args_size != 1) return badcommand();
		return quit();

	} else if (strcmp(command_args[0], "set")==0) {
		//set
		char *full_value;
		char *space = " ";

		if (args_size < 3) return badcommand();
		if (args_size > 7) return badcommandTooManyTokens();	//if more than 5 tokens
		full_value = command_args[2];
		strcat(full_value, space);
		for(int i=3; i<args_size; i++){ //concatenate all arguments + spaces in between to new pointer to pass to set
			strcat(full_value, command_args[i]);
			if(i!=args_size-1){
				strcat(full_value, space);
			}
		}
		return set(command_args[1], full_value);
	
	} else if (strcmp(command_args[0], "print")==0) {
		if (args_size != 2) return badcommand();
		return print(command_args[1]);
	
	} else if (strcmp(command_args[0], "run")==0) {
		if (args_size != 2) return badcommand();
		return run(command_args[1]);
	

	} else if (strcmp(command_args[0], "my_ls") == 0) {
		if (args_size != 1) return badcommand();
		return my_ls();

	} else if(strcmp(command_args[0], "echo")==0){
		if (args_size != 2) return badcommand();
		return echo(command_args[1]);
	}else return badcommand();

}

int help(){

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n \
my_ls			Lists all the files in the current directory\n ";
	printf("%s\n", help_string);
	return 0;
}

int quit(){
	printf("%s\n", "Bye!");
	exit(0);
}

int badcommand(){
	printf("%s\n", "Unknown Command");
	return 1;
}

// For run command only
int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}

// For set command only (more than 5 tokens)
int badcommandTooManyTokens(){
	printf("%s\n", "Bad command: Too many tokens");
	return 1;
}

int set(char* var, char* value){ 
	
	//char *link = "=";
	//char buffer[1000]; 
	//strcpy(buffer, var);
	//strcat(buffer, link);
	//strcat(buffer, value);
	mem_set_value(var, value);

	return 0;

}

int print(char* var){

	printf("%s\n", mem_get_value(var)); 
	return 0;
}

int run(char* script){
	int errCode = 0;
	char line[1000];
	FILE *p = fopen(script,"rt");  // the program is in a file

	if(p == NULL){
		return badcommandFileDoesNotExist();
	}

	fgets(line,999,p);
	while(1){
		errCode = parseInput(line);	// which calls interpreter()
		memset(line, 0, sizeof(line));

		if(feof(p)){
			break;
		}
		fgets(line,999,p);
	}

    fclose(p);

	return errCode;
}


int my_ls() {
	return system("ls -1");
}  

int echo(char *value){
	char *symbol = "$";

	if(value[0]==*symbol){ //check if argument starts with $
		if(strcmp(mem_get_value(value+1), "Variable does not exist")==0){
			printf("\n"); //if variable does not exist
		}else{
			print(value+1);
		}
	}else{
		printf("%s\n", value);
	}

	return 0;
}
