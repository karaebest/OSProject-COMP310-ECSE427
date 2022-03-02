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
int mem_set_value(char *var_in, char *value_in, int index) { //if index != -1, store var + value at index
	int i;
	if(index==-1){
			for (i=0; i<1000; i++){ 
				if (strcmp(shellmemory[i].var, var_in) == 0){
					shellmemory[i].value = strdup(value_in);
					return i;
				} 
			}

		//Value does not exist, need to find a free spot.
		for (i=0; i<1000; i++){
			if (strcmp(shellmemory[i].var, "none") == 0){
				shellmemory[i].var = strdup(var_in);
				shellmemory[i].value = strdup(value_in);
				return i;
			} 
		}

		return -1;
	}else{ //store value at index, no change to var at that index
		if (strcmp(shellmemory[index].var, var_in) == 0){
				shellmemory[index].value = strdup(value_in);
				return index;
		} 
		if (strcmp(shellmemory[index].var, "none") == 0){
				//shellmemory[index].var = strdup(var_in);
				shellmemory[index].value = strdup(value_in);
				return index;
			}

		return -1; //if not set (shell mem full)
	}
	

}

//get value based on input key + index difference from key
char *mem_get_value(char *var_in, int index) { 

	int i;

	for (i=0; i<1000; i++){
		if(i+index>1000){return "Index out of bounds";} 
		if (strcmp(shellmemory[i].var, var_in) == 0){
			return strdup(shellmemory[i+index].value);
		} 
	}
	return "Variable does not exist"; 

}