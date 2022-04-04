#include<stdlib.h>
#include<string.h>
#include<stdio.h>

// frame_size needs to be a multiple of 3
#define frame_size FSIZE
#define variable_size VSIZE

struct memory_struct{
	char *var;
	char *value;
};

struct memory_struct framestore[frame_size];
struct memory_struct variablestore[variable_size];

// Shell memory functions

void mem_reset_variable() {
	int i;
	for (i=0; i<variable_size; i++) {		
		variablestore[i].var = "none";
		variablestore[i].value = "none";
	}
}

void mem_init(){
	int i;
	for (i=0; i<frame_size; i++) {		
		framestore[i].var = "none";
		framestore[i].value = "none";
	}
	mem_reset_variable();
}

// finds the next hole available in memory and loads n lines from file p with offset of o lines
int mem_frame_load_next(FILE *p, int o, int n) {
	// go through offset of o lines
	for (int i = 0; i < o; i++) {
		char line[100];
		fgets(line,99,p);
	}
	for (int i = 0; i < frame_size; i+=3) {
		// if hole found
		if (strcmp(framestore[i].value, "none") == 0) {
			char line[100];
			// load n lines
			for (int j = 0; j < 3; j++) {						
				fgets(line,99,p);
				framestore[i + j].value = strdup(line);
				memset(line, 0, sizeof(line));
			}

			return i;
		}
	}
	return -1; //if not set (shell mem full)
}

//deletes frame starting at specified index (index will always be multiple of 3 within frame store size)
void mem_frame_delete(int index){
	for(int i=0; i<3; i++){
		framestore[index + i].value = "none";
		framestore[index + i].var = "none";
	}
}

// Set key value pair at index (unless index == -1, then stored at next free space) in frame store
int mem_frame_set_value(char *var_in, char *value_in, int index) { 
	
	if(index==-1){
		int i;
		for (i=0; i<frame_size; i++){ 
			if (strcmp(framestore[i].var, var_in) == 0){
				framestore[i].value = strdup(value_in);
				return i;
			} 
		}

		//Value does not exist, need to find a free spot.
		for (i=0; i<frame_size; i++){
			if (strcmp(framestore[i].value, "none") == 0){ //must check value instead of var since script lines after first have var = "none"
				framestore[i].var = strdup(var_in);
				framestore[i].value = strdup(value_in);
				return i;
			} 
		}

		return -1;
	}else{ //store value at index, no change to var at that index (for script loading, first instruction's var = name of script, following instructions' var = none)
		if (strcmp(framestore[index].var, var_in) == 0){
			framestore[index].value = strdup(value_in);
			return index;
		} 
		if (strcmp(framestore[index].var, "none") == 0){
			//framestore[index].var = strdup("last");
			framestore[index].value = strdup(value_in);
			return index;
		}

		return -1; //if not set (shell mem full)
	}
}

// Set key value pair at index (unless index == -1, then stored at next free space) in variable store
int mem_variable_set_value(char *var_in, char *value_in, int index) { 
	
	if(index==-1){
		int i;
		for (i=0; i<variable_size; i++){ 
			if (strcmp(variablestore[i].var, var_in) == 0){
				variablestore[i].value = strdup(value_in);
				return i;
			} 
		}

		//Value does not exist, need to find a free spot.
		for (i=0; i<variable_size; i++){
			if (strcmp(variablestore[i].value, "none") == 0){ //must check value instead of var since script lines after first have var = "none"
				variablestore[i].var = strdup(var_in);
				variablestore[i].value = strdup(value_in);
				return i;
			} 
		}

		return -1;
	}else{ //store value at index, no change to var at that index (for script loading, first instruction's var = name of script, following instructions' var = none)
		if (strcmp(variablestore[index].var, var_in) == 0){
			variablestore[index].value = strdup(value_in);
			return index;
		} 
		if (strcmp(variablestore[index].var, "none") == 0){
			//variablestore[index].var = strdup("last");
			variablestore[index].value = strdup(value_in);
			return index;
		}

		return -1; //if not set (shell mem full)
	}
}

//get frame value based on input key + index difference from key (i of returned value = i of var_in + index_in)
char *mem_frame_get_value(char *var_in, int index_in) { //if var_in = NULL, return value at index_in

	if(var_in == NULL){
		return strdup(framestore[index_in].value); 
	}
	for (int i=0; i<frame_size; i++){
		if(i+index_in>frame_size){return "Index out of bounds";} 
		if (strcmp(framestore[i].var, var_in) == 0){
			return strdup(framestore[i+index_in].value);
		} 
	}
	return "Frame does not exist"; 
}

//get variable value based on input key + index difference from key (i of returned value = i of var_in + index_in)
char *mem_variable_get_value(char *var_in, int index_in) { //if var_in = NULL, return value at index_in

	if(var_in == NULL){
		return strdup(variablestore[index_in].value); 
	}
	for (int i=0; i<variable_size; i++){
		if(i+index_in>variable_size){return "Index out of bounds";} 
		if (strcmp(variablestore[i].var, var_in) == 0){
			return strdup(variablestore[i+index_in].value);
		} 
	}
	return "Variable does not exist"; 
}

// int mem_check_for_script(char *var_in){		//returns start index of script if already in mem, -1 if it isn't
// 	for(int i=0; i<1000; i++){
// 		if(strcmp(shellmemory[i].var, var_in) == 0){
// 			return i;
// 		}
// 	}
// 	return -1;
// }