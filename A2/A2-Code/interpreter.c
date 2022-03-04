#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "shellmemory.h"
#include "shell.h"
#include "PCB.h"

int MAX_ARGS_SIZE = 7;

int help();
int quit();
int badcommand();
int badcommandTooManyTokens();
int badcommandFileDoesNotExist();
int set(char* var, char* value);
int print(char* var);
int run(char* script, char* policy);
int my_ls();
int echo();
int exec(char scripts[], char* policy, int num_scripts);

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
		if (args_size != 3) return badcommand(); 
		return run(command_args[1], command_args[2]); 
	
	} else if (strcmp(command_args[0], "my_ls")==0) {
		if (args_size > 2) return badcommand();
		return my_ls();
	
	}else if (strcmp(command_args[0], "echo")==0) {
		if (args_size > 2) return badcommand();
		return echo(command_args[1]);
	
	}else if(strcmp(command_args[0], "exec")==0){  
		if(args_size < 3) return badcommand();
		char script_list[args_size-2]; 		//TO DO -> make sure no memory leaks
		for(i=1; i<args_size-1; i++){
			strcpy(script_list[i-1], command_args[i]);
		}
		return exec(script_list, command_args[args_size-1], args_size-2);
	}
	else return badcommand();
}

int help(){

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
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

int badcommandTooManyTokens(){
	printf("%s\n", "Bad command: Too many tokens");
	return 2;
}

int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}

int set(char* var, char* value){

	// char *link = "=";
	// char buffer[1000];
	// strcpy(buffer, var);
	// strcat(buffer, link);
	// strcat(buffer, value);

	return mem_set_value(var, value, -1);
}

int print(char* var){
	printf("%s\n", mem_get_value(var, 0)); 
	return 0;
}

int run(char* script, char* policy){ 
	char line[100];
	char* name_script = (char*)malloc(sizeof(script)-4);
	strncpy(name_script, script, sizeof(script)-4);

	FILE *p = fopen(script,"rt");

	if(p == NULL){
		return badcommandFileDoesNotExist();
	}

	int length = 0;
	int index; 

	fgets(line,99,p);
	while(1){
		length++;
		index = mem_set_value(name_script, line, index) + 1;
		memset(line, 0, sizeof(line));
		if(feof(p)){
			break;
		}
		fgets(line,99,p);
	}

	fclose(p);
	free(name_script);
	int start = index - length;
	int errCode = scheduler(length, start, -1, NULL); //TO DO -> change null to it at scheduler call
	return errCode;
}

int my_ls(){
	int errCode = system("ls | sort");
	return errCode;
}

int echo(char* var){
	if(var[0] == '$'){
		var++;
		printf("%s\n", mem_get_value(var, 0)); 
	}else{
		printf("%s\n", var); 
	}
	return 0; 
}

int exec(char scripts[], char* policy, int num_scripts){
	char line[100];
	char* name_script;
	int errCode;
	int start;
	int length;
	int index;
	
	for(int i=0; i<num_scripts; i++){
		name_script = (char*)malloc(sizeof(scripts[i])-4);
		strncpy(name_script, scripts[i], sizeof(scripts[i])-4);

		FILE *p = fopen(scripts[i],"rt");

		if(p == NULL){
			return badcommandFileDoesNotExist();
		}

		length = 0; 

		fgets(line,99,p);
		while(1){
			length++;
			index = mem_set_value(name_script, line, index) + 1;
			memset(line, 0, sizeof(line));
			if(feof(p)){
				break;
			}
			fgets(line,99,p);
		}

		fclose(p);
		free(name_script);
		start = index - length;

		if(i!=num_scripts-1) errCode = scheduler(length, start, 1, NULL);
	}
	return errCode = scheduler(length, start, -1, NULL);
}