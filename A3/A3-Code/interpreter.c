#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "shellmemory.h"
#include "shell.h"
#include "PCB.h"

#define frame_size FSIZE
#define var_size VSIZE

int MAX_ARGS_SIZE = 7;

int help();
int quit();
int badcommand();
int badcommandTooManyTokens();
int badcommandFileDoesNotExist();
int badcommandSameFileName();
int set(char* var, char* value);
int print(char* var);
int run(char* script, char* policy, int multi);
int my_ls();
int echo(char* var);
int resetmem();

int interpreter(char* command_args[], int args_size){
	int i;

	if ( args_size < 1 || args_size > MAX_ARGS_SIZE){
		if (strcmp(command_args[0], "set")==0 && args_size > MAX_ARGS_SIZE) {
			return badcommandTooManyTokens();
		}
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
		if (args_size < 3) return badcommand();
		char* value = (char*)calloc(1,150);
		char spaceChar = ' ';

		for(int i = 2; i < args_size; i++){
			strncat(value, command_args[i], 30);
			if(i < args_size-1){
				strncat(value, &spaceChar, 1);
			}
		}
		set(command_args[1], value);
		return 0;
	
	} else if (strcmp(command_args[0], "print")==0) {
		if (args_size != 2) return badcommand();
		return print(command_args[1]);
	
	} else if (strcmp(command_args[0], "run")==0) {
		if (args_size != 2) return badcommand(); 
		return run(command_args[1], "FCFS", 0);
	
	} else if (strcmp(command_args[0], "my_ls")==0) {
		if (args_size != 1) return badcommand();
		return my_ls();
	
	}else if (strcmp(command_args[0], "echo")==0) {
		if (args_size > 2) return badcommand();
		return echo(command_args[1]);
	
	}else if(strcmp(command_args[0], "exec")==0){  
		if(args_size < 3 || args_size > 5) return badcommand();

		for(int i = 1; i<args_size-1; i++){
			if(i==args_size-2) return run(command_args[i], command_args[args_size-1], 0);
			run(command_args[i], command_args[args_size-1], 1);
		}
		return 0;
		
	} else if (strcmp(command_args[0], "resetmem")==0) {
		if (args_size != 1) return badcommand();
		return resetmem();
	}
	else return badcommand();
}

int help(){

	char help_string[] = "COMMAND			DESCRIPTION\n \
	help			Displays all the commands\n \
	quit			Exits / terminates the shell with “Bye!”\n \
	set VAR STRING		Assigns a value to shell memory\n \
	print VAR		Displays the STRING assigned to VAR\n \
	run SCRIPT.TXT		Executes the file SCRIPT.TXT\n \
	echo STRING		Displays STRING\n \
	my_ls			Lists all the files in the current directory\n \
	resetmem		Deletes the content of the variable store\n	\
	Frame Store Size = ";

	char buffer[1000];
	char *numF;
	char *numV;
	strcpy(buffer, help_string);
	asprintf(&numF, "%d", frame_size);
	strcat(buffer, numF);
	strcat(buffer, "; Variable Store Size = ");
	asprintf(&numV, "%d", var_size);
	strcat(buffer, numV);
	strcat(buffer, "\n");


	printf("%s\n", buffer);
	free(numF);
	free(numV);
	return 0;
}

int quit(){
	printf("%s\n", "Bye!");
	system("exec rm -r backing_store");
	exit(0);
}

int badcommand(){
	printf("%s\n", "Unknown Command");
	return 1;
}

int badcommandTooManyTokens(){
	printf("%s\n", "Bad command: Too many tokens");
	return 2;
}

int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}

int badcommandSameFileName(){
	printf("%s\n", "Bad command: Same file name");
	return 4;
}



int set(char* var, char* value){

	// char *link = "=";
	// char buffer[1000];
	// strcpy(buffer, var);
	// strcat(buffer, link);
	// strcat(buffer, value);

	return mem_variable_set_value(var, value, -1);
}

int print(char* var){
	printf("%s\n", mem_variable_get_value(var, 0)); 
	return 0;
}

int run(char* script, char* policy, int multi){ 
	char line[100];
	char* name_script = malloc(sizeof(script));
	name_script = strncpy(name_script, script, sizeof(script));
	char command[50];
	sprintf(command, "exec cp %s backing_store/%s", name_script, name_script);
	system(command);

	FILE *p = fopen(script,"rt");

	if(p == NULL){
		return badcommandFileDoesNotExist();
	}

	int length = 0;

	fgets(line,99,p);
	// count script length
	while(1){
		length++;
		memset(line, 0, sizeof(line));
		if(feof(p)){
			break;
		}
		fgets(line,99,p);
	}

	fclose(p);
	free(name_script);
	// call scheduler for this script
	int errCode = scheduler(script, length, multi, policy);
	return errCode;
}

int my_ls(){
	int errCode = system("ls | sort");
	return errCode;
}

int echo(char* var){
	if(var[0] == '$'){
		var++;
		printf("%s\n", mem_variable_get_value(var, 0)); 
	}else{
		printf("%s\n", var); 
	}
	return 0; 
}

int resetmem() {
	mem_reset_variable();
	return 0;
}

