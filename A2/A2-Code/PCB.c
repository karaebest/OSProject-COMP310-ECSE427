#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "shellmemory.h"
#include "shell.h"


typedef struct PCB_struct{
    int pid;
    int counter; //index difference from script starting point in shell mem (start+counter = index of instruction being executed in shell mem)
    int start;//starting index in shell memory
    int length;//num lines of script
    struct PCB_struct* next;//pointer to next PCB
     
} PCB_t;

//TO DO --> change variables in h files
//TO DO --> add error messages

static PCB_t* head = NULL;//pointer to head of ready queue

void end_process();
int run_process();

int scheduler(int len, int start, int multi, char* policy){ //begin process (append to end of ready queue), return pid, will need to change for multi-progr.
    //for multi FCFS: add bool multi as arg, if 1 return w/o running after adding to queue, in interpreter change to false for last prog
    int pid_counter = 1; //to make sure new processes have unique pid, might have to change this later

    if(head == NULL){
        head = (PCB_t *)malloc(sizeof(PCB_t));  
        head->pid = pid_counter;
        head->counter = 0;  
        head->start = start;
        head->length = len;
        head->next = NULL;
    }else{
        PCB_t *current = head;
    
        while (current->next != NULL)
        {
            pid_counter = current->pid;
            current = current->next;
        }

        current->next = (PCB_t *)malloc(sizeof(PCB_t));
        current->next->pid = ++pid_counter;
        current->next->counter = 0;  
        current->next->start = start;
        current->next->length = len;
        current->next->next = NULL;
    }
    if(multi==1) return 0;

    return run_process();
}

int run_process(){
    int errCode;
    while((head->counter)!=(head->length)){
        errCode = parseInput(mem_get_value(NULL, head->start+head->counter));
        head->counter++;
    }
    end_process();
    return errCode;
}

void end_process() //no need to check for empty linked list because will never be called in that case
{
    if(head->next==NULL){
        mem_delete_script(head->start, head->length); //delete script code from shell mem
        free(head);
        head = NULL;
    }else{
        PCB_t* next_p = head->next;
        free(head);
        head = next_p;
    }
}

