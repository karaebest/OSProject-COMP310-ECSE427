#include <stdio.h>
#include <stdlib.h>
#include <string.h> 


struct PCB_struct{
    const int pid;
    char* counter; //current instruction to execute, points at script line in shell memory
    
    int start;//starting index in shell memory
    int length;//length or last index in shell memory
    struct PCB_struct* next;//pointer to next PCB
    struct PCB_struct* head;//pointer to head of ready queue (static?) 
};
//must dyn allocate PCB

struct PCB_struct ready_queue[3]; //can contain at most 3 processes

void queue_init(){
    for(int i=0; i<3; i++){
        ready_queue[i].pid = -1;
        ready_queue[i].counter = "none";
        ready_queue[i].start = -1;
        ready_queue[i].length = -1;
        ready_queue[i].next = NULL;
        ready_queue[i].head = NULL;

    }
}

void begin_process(int start_index, int length){
    
    for(int i=0; i<3; i++){
        if(ready_queue[i].pid == -1){

        }
    }
}