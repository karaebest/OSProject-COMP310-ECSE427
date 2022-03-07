#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "shellmemory.h"
#include "shell.h"

typedef struct PCB_struct{
    int pid;
    int counter; //index difference from script starting point in shell mem (start+counter = index of instruction being executed in shell mem)
    int start; //starting index in shell memory
    int length; //num lines of script
    int estimate; //estimate length for aging
    struct PCB_struct* next; //pointer to next PCB
     
} PCB_t;

//TO DO --> change variables in h files
//TO DO --> add error messages

static PCB_t* head = NULL;//pointer to head of ready queue

int end_process();
int run_process();
void end_all_process();
void promote_process(int);
void age_all_process();

int scheduler(int len, int start, int multi, char* policy){ //begin process (append to end of ready queue), return pid
    //for multi FCFS: add bool multi as arg, if 1 return w/o running after adding to queue, if 0 run processes after adding
    int pid_counter = 1; //to make sure new processes have unique pid

    if(head == NULL){
        head = (PCB_t *)malloc(sizeof(PCB_t));  
        head->pid = pid_counter;
        head->counter = 0;  
        head->start = start;
        head->length = len;
        head->estimate = len;
        head->next = NULL;
    }else{
        PCB_t *current = head;
    
        while (current->next != NULL)
        {
            current = current->next;
        }
        pid_counter = current->pid;

        current->next = (PCB_t *)malloc(sizeof(PCB_t));
        current->next->pid = ++pid_counter;
        current->next->counter = 0;  
        current->next->start = start;
        current->next->length = len;
        current->next->estimate = len;
        current->next->next = NULL;
    }
    if(multi==1) return 0;

    return run_process(policy);
}

int run_process(char* policy){
    int errCode;
    PCB_t *current = head;
    if (strcmp(policy, "FCFS") == 0) { // first come first serve policy
        while ((head != NULL)) {
            while((head->counter)!=(head->length)){ // run all instructions of script
                errCode = parseInput(mem_get_value(NULL, head->start+head->counter));
                if (errCode != 0) { // exit upon error
                    end_all_process();
                    return errCode;
                }
                head->counter++;
            }
            end_process(head->pid); // end process when done
        }
    }
    else if (strcmp(policy, "RR") == 0) { // round robin policy
        while ((current != NULL)) {
            int diff = current->length - current->counter;
            if (diff <= 0) { // if process is done, end it and go to next once
                PCB_t* next_p = current->next;
                end_process(current->pid);
                current = next_p;
                if (current == NULL) current = head;
                continue;
            }
            else if (diff > 2){ // limit execution to 2 instructions
                diff = 2;
            }
            for (int i=0; i < diff; i++) {
                errCode = parseInput(mem_get_value(NULL, current->start+current->counter));
                if (errCode != 0) { // exit upon error
                    end_all_process();
                    return errCode;
                }
                current->counter++;
            }
            if (current->next == NULL) current = head; // if at end of linked list, go back to head
            else current = current->next; // go to next node
        }
    }
    else if (strcmp(policy, "SJF") == 0) { // shortest job first policy
        while (head != NULL) {
            int minlength = -1;
            PCB_t *minjob = head;
            current = head;
            while (current != NULL) { // find shortest job
                if (minlength > current->length || minlength == -1) {
                    minjob = current;
                    minlength = minjob->length;
                }
                current = current->next;
            }
            while ((minjob->counter) != (minjob->length)){ // run all instructions of script
                errCode = parseInput(mem_get_value(NULL, minjob->start+minjob->counter));
                if (errCode != 0) { // exit upon error
                    end_all_process();
                    return errCode;
                }
                minjob->counter++;
            }
            end_process(minjob->pid); // end process when done
        }
    }
    else if (strcmp(policy, "AGING") == 0) {
        while (head != NULL) {
            int minestimate = -1;
            PCB_t *minjob = head;
            current = head;
            while (current != NULL) { // find minimum estimate job
                if (minestimate > current->estimate || minestimate == -1) {
                    minjob = current;
                    minestimate = minjob->estimate;
                }
                current = current->next;
            }
            if (minjob != head) {
                promote_process(minjob->pid); // promote job
            }
            int diff = (head->length) - (head->counter);
            if (diff > 0){ // run 1 instruction
                errCode = parseInput(mem_get_value(NULL, head->start+head->counter));
                if (errCode != 0) { // exit upon error
                    end_all_process();
                    return errCode;
                }
                head->counter++;
            }
            if (diff == 1) { // if no more instructions, end process
                end_process(head->pid);
            }
            age_all_process(); // decrease estimate by 1 for every process after head
        }
    }
    return errCode;
}

// helper function promote a process, and put head at the end of queue
void promote_process(int pid) {
    // put head process to last
    PCB_t* last = head;
    while (last->next!= NULL) {
        last = last->next;
    }
    PCB_t* temp = head->next;
    last->next = head;
    head->next = NULL;
    head = temp;
    // promote other process to head
    if (head->pid != pid) {
        PCB_t* previous = head;
        PCB_t* current = head->next;
        while (current->pid != pid) {
            previous = current;
            current = current->next;
        }
        previous->next = current->next;
        current->next = head;
        head = current;
    }
}

// helper function to reduce estimate by 1 for every process after head
void age_all_process() {
    if (head != NULL) {
        PCB_t* current = head->next;
        while (current != NULL) {
            if (current->estimate > 0) current->estimate--; // decrease if estimate is nonzero
            current = current->next;
        }
    }
}

// helper function to end a process by pid
int end_process(int pid) //no need to check for empty linked list because will never be called in that case
{
    // if deleting the head node
    if(head->pid==pid){
        mem_delete_script(head->start, head->length); //delete script code from shell mem
        if (head->next != NULL) {
            PCB_t* next_p = head->next;
            free(head);
            head = next_p;
        }
        else {
            free(head);
            head = NULL;
        }
        return 0;
    // if deleting another node
    } else {
        PCB_t* previous = head;
        PCB_t* current = head->next;
        while (current->pid != pid) {
            previous = current;
            current = current->next;
        }
        previous->next = current->next;
        mem_delete_script(current->start, current->length); //delete script code from shell mem
        free(current);
        return 0;
    }
    return -1;
}

// helper function to end all processes
void end_all_process() { // if an issue arose during execution, delete the PCB linked list and free all memory
    while (head != NULL) {
        mem_delete_script(head->start, head->length); //delete script code from shell mem
        PCB_t* next_p = head->next;
        free(head);
        head = next_p;
    }
}

