#include<stdlib.h>
#include<string.h>
#include<stdio.h>


struct memory_struct{
	char *var;
	char *value;
};

struct memory_struct shellmemory[1000];

// Shell memory functions

void mem_init(){

	int i;
	for (i=0; i<1000; i++){		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}

// Set key value pair at index (unless index == -1, then stored at next free space)
int mem_set_value(char *var_in, char *value_in, int index) { 
	
	if(index==-1){
		int i;
		for (i=0; i<1000; i++){ 
			if (strcmp(shellmemory[i].var, var_in) == 0){
				shellmemory[i].value = strdup(value_in);
				return i;
			} 
		}

		//Value does not exist, need to find a free spot.
		for (i=0; i<1000; i++){
			if (strcmp(shellmemory[i].value, "none") == 0){ //must check value instead of var since script lines after first have var = "none"
				shellmemory[i].var = strdup(var_in);
				shellmemory[i].value = strdup(value_in);
				return i;
			} 
		}

		return -1;
	}else{ //store value at index, no change to var at that index (for script loading, first instruction's var = name of script, following instructions' var = none)
		if (strcmp(shellmemory[index].var, var_in) == 0){
			shellmemory[index].value = strdup(value_in);
			return index;
		} 
		if (strcmp(shellmemory[index].var, "none") == 0){
			//shellmemory[index].var = strdup("last");
			shellmemory[index].value = strdup(value_in);
			return index;
		}

		return -1; //if not set (shell mem full)
	}
	

}

//get value based on input key + index difference from key (i of returned value = i of var_in + index_in)
char *mem_get_value(char *var_in, int index_in) { //if var_in = NULL, return value at index_in

	if(var_in == NULL){
		return strdup(shellmemory[index_in].value); //add error message here if index_in is outOfBounds
	}
	for (int i=0; i<1000; i++){
		if(i+index_in>1000){return "Index out of bounds";} 
		if (strcmp(shellmemory[i].var, var_in) == 0){
			return strdup(shellmemory[i+index_in].value);
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