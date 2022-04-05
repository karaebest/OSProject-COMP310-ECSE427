#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shellmemory.h"
#include "shell.h"

typedef struct PCB_struct{
    char* script;
    int pid;
    int counter; //index difference from script starting point in shell mem (start+counter = index of instruction being executed in shell mem)
    int start; //starting index in shell memory
    int length; //num lines of script
    int estimate; //estimate length for aging
    int pagenumber; // current page number to indentify in pagetable
    int pagetable[10]; // pagetable holding framstore index of each page
    // TODO: change page table size. Assume each script will have no more than 30 lines for now
    struct PCB_struct* next; //pointer to next PCB
     
} PCB_t;

//TO DO --> change variables in h files
//TO DO --> add error messages

static PCB_t* head = NULL;//pointer to head of ready queue

static int counter = 0;

int end_process();
int run_process();
void load_processes();
void end_all_process();
void promote_process(int, int);
void age_all_process();

int scheduler(char* name, int len, int multi, char* policy){ //begin process (append to end of ready queue), return pid
    //for multi FCFS: add bool multi as arg, if 1 return w/o running after adding to queue, if 0 run processes after adding
    int pid_counter = 1; //to make sure new processes have unique pid

    if(head == NULL){
        head = (PCB_t *)malloc(sizeof(PCB_t));
        head->script = name;
        head->pid = pid_counter;
        head->counter = 0;  
        // head->start = start;
        head->length = len;
        head->estimate = len;
        head->pagenumber = 0;
        head->next = NULL;
    }else{
        PCB_t *current = head;
    
        while (current->next != NULL)
        {
            current = current->next;
        }
        pid_counter = current->pid;

        current->next = (PCB_t *)malloc(sizeof(PCB_t));
        current->next->script = name;
        current->next->pid = ++pid_counter;
        current->next->counter = 0;  
        // current->next->start = start;
        current->next->length = len;
        current->next->estimate = len;
        current->next->pagenumber = 0;
        current->next->next = NULL;
    }
    if(multi==1) return 0;
    load_processes();
    return run_process(policy);
}

    FILE *p = fopen(path,"rt");
    if(p == NULL){ // file cannot be opened      MIGHT NEED TO CHANGE THIS
        end_all_process();
        exit(99);
    }

    // load page at next free hole in memory.
    int index = mem_frame_load_next(p, (page_number*3), lines, counter);
    counter++;
    if (index == -1) {              // no more free memory
        fclose(p);
        free(name_script);
        return index;
    }
    current->pagetable[page_number] = index; // here, index is the framestore index where the page is saved
    fclose(p);
    free(name_script);
    return 0;
}

// load program pages alternatingly into the frame store. each page is 3 lines
// void load_processes() {
//     PCB_t *current = head;
//     int max = 0;
//     // find script with the most lines, and loop for number of lines / 3
//     while (current != NULL) {
//         int n = (current->length + 2) / 3; // round up
//         if (n > max) {
//             max = n;
//         }
//         current = current->next;
//     }
//     // loop max times to go through all pages of all scripts
//     for (int i = 0; i < max; i++) {
//         current = head; // go back to head for next pages
        
        // for each script, load the next 3 lines into memory at the next free hole
        while (current != NULL ) {
            int loads = 3;
            // if script is finished, go to next
            if (current->length == current->counter) {
                current = current->next;
                continue;
            }
            // if less than 3 lines left, only loop for what's left
            else if (current->length - current->counter < 3) {
                loads = current->length - current->counter;
            }
            char* name_script = malloc(sizeof(current->script));
            name_script = strncpy(name_script, current->script, sizeof(current->script));
            char path[50];
            sprintf(path, "backing_store/%s", name_script);

            FILE *p = fopen(path,"rt");
            if(p == NULL){ // file cannot be opened
                end_all_process();
                exit(99);
            }

        int index = mem_frame_find_lru();
        //TO ADD HERE: finding
        char* evict = "\nPage fault! Victim page contents:\n";
        char buffer[400]; // each line is no longer than 100 characters
        strcpy(buffer, evict);
        for(int i=0; i<3; i++){
            if(strcmp(mem_frame_get_line(index+i), "none")!=0){
                strcat(buffer, mem_frame_get_line(index+i));
                // strcat(buffer, "\n");       // might need to add extra \ before newline
            }
            current->pagetable[i] = index; // here, index is the framestore index where the page is saved

            fclose(p);
            free(name_script);
            current->counter += loads;
            current = current->next;
        }
        strcat(buffer, "End of victim page contents.\n");
        printf("%s\n", buffer);
        mem_frame_delete(index);            //delete frame from store
        int res = load_page(process, process->pagenumber);
        if (res == -1) {
            printf("ERROR big prob");
        }
    }
    // reset counters to 0
    current = head;
    while (current != NULL) {
        current->counter = 0;
        current = current->next;
    }
}


int run_process(char* policy){
    int errCode;
    PCB_t *current = head;
    if (strcmp(policy, "FCFS") == 0) { // first come first serve policy
        while ((head != NULL)) {
            while((head->counter)!=(head->length)){ // run all instructions of script
                int index = head->pagetable[head->pagenumber] + head->counter % 3; // index is index pointed to by page table + offset from the counter
                errCode = parseInput(mem_frame_get_value(NULL, index, counter));
                counter++;
                if (errCode != 0) { // exit upon error
                    end_all_process();
                    return errCode;
                }
                head->counter++;
                if (head->counter % 3 == 0) { // when counter is multiple of 3, increase page number to go to next page
                    head->pagenumber++;
                }
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
                int index = current->pagetable[current->pagenumber] + current->counter % 3; // index is index pointed to by page table + offset from the counter
                errCode = parseInput(mem_frame_get_value(NULL, index, counter));
                counter++;
                if (errCode != 0) { // exit upon error
                    end_all_process();
                    return errCode;
                }
                current->counter++;
                if (current->counter % 3 == 0) {  // when counter is multiple of 3, increase page number to go to next page
                    current->pagenumber++;
                }
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
                int index = minjob->pagetable[minjob->pagenumber] + minjob->counter % 3; // index is index pointed to by page table + offset from the counter
                errCode = parseInput(mem_frame_get_value(NULL, index, counter));
                counter++;
                if (errCode != 0) { // exit upon error
                    end_all_process();
                    return errCode;
                }
                minjob->counter++;
                if (minjob->counter % 3 == 0) {  // when counter is multiple of 3, increase page number to go to next page
                    minjob->pagenumber++;
                }
            }
            end_process(minjob->pid); // end process when done
        }
    }
    else if (strcmp(policy, "AGING") == 0) {
        int flag = 0; // flag to know if need to move head node to tail
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
                promote_process(minjob->pid, flag); // promote job
                flag++;
            }
            int diff = (head->length) - (head->counter);
            if (diff > 0){ // run 1 instruction
                flag++;
                int index = head->pagetable[head->pagenumber] + head->counter % 3; // index is index pointed to by page table + offset from the counter
                errCode = parseInput(mem_frame_get_value(NULL, index, counter));
                counter++;
                if (errCode != 0) { // exit upon error
                    end_all_process();
                    return errCode;
                }
                head->counter++;
                if (head->counter % 3 == 0) {  // when counter is multiple of 3, increase page number to go to next page
                    head->pagenumber++;
                }
            }
            age_all_process(); // decrease estimate by 1 for every process after head
            if (diff == 1) { // if no more instructions, end process
                end_process(head->pid);
            }
        }
    }
    end_all_process();
    return errCode;
}

// helper function promote a process, and put head at the end of queue
void promote_process(int pid, int flag) {
    if (flag != 0) { // don't move head to tail if there have been no promotions and no instructions executed yet
        // put head process to last
        PCB_t* last = head;
        while (last->next!= NULL) {
            last = last->next;
        }
        PCB_t* temp = head->next;
        last->next = head;
        head->next = NULL;
        head = temp;
    }
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
        free(current);
        return 0;
    }
    return -1;
}

// helper function to end all processes
void end_all_process() { // if an issue arose during execution, delete the PCB linked list and free all memory
    while (head != NULL) {
        PCB_t* next_p = head->next;
        free(head);
        head = next_p;
    }
}

