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

PCB_t* end_process();
int run_process();
void end_all_process();

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

    return run_process(policy);
}

int run_process(char* policy){
    int errCode;
    PCB_t *current = head;
    if (strcmp(policy, "FCFS") == 0) {
        while ((head != NULL)) {
            while((head->counter)!=(head->length)){
                errCode = parseInput(mem_get_value(NULL, head->start+head->counter));
                if (errCode != 0) {
                    end_all_process();
                    return errCode;
                }
                head->counter++;
            }
            end_process(head->pid);
        }
    }
    else if (strcmp(policy, "RR") == 0) {
        while ((current != NULL)) {
            int diff = current->length - current->counter;
            // printf("current pid: %d, current diff: %d\n", current->pid, diff);
            if (diff <= 0) {
                current = end_process(current->pid);
                if (current == NULL) current = head;
                continue;
            }
            else if (diff > 2){
                diff = 2;
            }
            for (int i=0; i < diff; i++) {
                errCode = parseInput(mem_get_value(NULL, current->start+current->counter));
                if (errCode != 0) {
                    end_all_process();
                    return errCode;
                }
                current->counter++;
            }
            if (current->next == NULL) current = head;
            else current = current->next;
        }
    }
    else if (strcmp(policy, "SJF") == 0) {
        int minlength = -1;
        while (head != NULL) {  
            PCB_t *minjob = head;
            current = head;
            while (current != NULL) {
                if (minlength > current->length || minlength == -1) {
                    minlength = current->length;
                    minjob = current;
                }
                current = current->next;
            }
            while((minjob->counter)!=(minjob->length)){
                errCode = parseInput(mem_get_value(NULL, minjob->start+minjob->counter));
                if (errCode != 0) {
                    end_all_process();
                    return errCode;
                }
                minjob->counter++;
            }
            end_process(minjob->pid);
        }
    }
    return errCode;
}

PCB_t* end_process(int pid) //no need to check for empty linked list because will never be called in that case
{
    if(head->pid==pid){
        mem_delete_script(head->start, head->length); //delete script code from shell mem
        PCB_t* next_p = head->next;
        free(head);
        head = next_p;
        return head;
    } else{
        PCB_t* previous = head;
        PCB_t* current = head->next;
        while (current->pid != pid) {
            previous = current;
            current = current->next;
        }
        previous->next = current->next;
        mem_delete_script(current->start, current->length); //delete script code from shell mem
        free(current);
        return previous->next;
    }
    return NULL;
}

void end_all_process() { // if an issue arose during execution, delete the PCB linked list and free all memory
    while (head != NULL) {
        mem_delete_script(head->start, head->length); //delete script code from shell mem
        PCB_t* next_p = head->next;
        free(head);
        head = next_p;
    }
}

