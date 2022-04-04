#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shellmemory.h"
#include "shell.h"

#define frame_size FSIZE

typedef struct PCB_struct{
    char* script;
    int pid;
    int counter; //index difference from script starting point in shell mem (start+counter = index of instruction being executed in shell mem)
    int start; //starting index in shell memory
    int length; //num lines of script
    int estimate; //estimate length for aging
    int pagenumber; // current page number to indentify in pagetable
    int pagetable[34]; // pagetable holding framstore index of each page, assumes script will be no longer than 100 lines (as stated on ed)
    struct PCB_struct* next; //pointer to next PCB
     
} PCB_t;


static PCB_t* head = NULL;//pointer to head of ready queue
static int *framestorepages[frame_size/3]; //holds array of pointers to pagetable locations of pages currently in framestore (index*3 = frame number)


int end_process();
int run_process();
void load_processes();
void end_all_process();
void promote_process(int, int);
void age_all_process();
int load_page(PCB_t *current, int page_number);
void page_fault(PCB_t *process);

int scheduler(char* name, int len, int multi, char* policy){ //begin process (append to end of ready queue), return pid
    //for multi FCFS: add bool multi as arg, if 1 return w/o running after adding to queue, if 0 run processes after adding
    int pid_counter = 1; //to make sure new processes have unique pid
    int i;
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
        for(i=0; i<34; i++){                //initialize all spots in pagetable to -1
            head->pagetable[i] = -1;
        }
        load_page(head, 0);       //load first 2 pages 
        if(head->length>3) load_page(head, 1);
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
        for(i=0; i<34; i++){           
            current->next->pagetable[i] = -1;
        }
        load_page(current->next, 0);   //load first 2 pages
        if(current->next->length > 3) load_page(current->next, 1);
        
    }
    
    if(multi==1) return 0;
    //load_processes();
    return run_process(policy);
}
 
int load_page(PCB_t *current, int page_number){ 
      
    int lines = 3;
    if (current->length - (page_number*3) < 3) {
        lines = current->length - (page_number*3);
    }
    char* name_script = malloc(sizeof(current->script));
    name_script = strncpy(name_script, current->script, sizeof(current->script));
    char path[100];
    sprintf(path, "backing_store/%s", name_script);

    FILE *p = fopen(path,"rt");
    if(p == NULL){ // file cannot be opened      MIGHT NEED TO CHANGE THIS
        end_all_process();
        exit(99);
    }

    // load page at next free hole in memory.
    int index = mem_frame_load_next(p, (page_number*3), lines);
    if (index == -1) {              // no more free memory
        fclose(p);
        free(name_script);
        return index;
    }
    current->pagetable[page_number] = index; // here, index is the framestore index where the page is saved
    framestorepages[index/3] = &(current->pagetable[page_number]);      
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
        
//         // for each script, load the next 3 lines into memory at the next free hole
//         while (current != NULL ) {
//             int loads = 3;
//             // if script is finished, go to next
//             if (current->length == current->counter) {
//                 current = current->next;
//                 continue;
//             }
//             // if less than 3 lines left, only loop for what's left
//             else if (current->length - current->counter < 3) {
//                 loads = current->length - current->counter;
//             }
//             char* name_script = malloc(sizeof(current->script));
//             name_script = strncpy(name_script, current->script, sizeof(current->script));
//             char path[100];
//             sprintf(path, "backing_store/%s", name_script);

//             FILE *p = fopen(path,"rt");
//             if(p == NULL){ // file cannot be opened
//                 end_all_process();
//                 exit(99);
//             }

//             // load page at next free hole in memory.
//             int index = mem_frame_load_next(p, current->counter, loads);
//             if (index == -1) { // no more free memory
//                 end_all_process();
//                 exit(99);
//             }
//             current->pagetable[i] = index; // here, index is the framestore index where the page is saved

//             fclose(p);
//             free(name_script);
//             current->counter += loads;
//             current = current->next;
//         }
//     }
//     // reset counters to 0
//     current = head;
//     while (current != NULL) {
//         current->counter = 0;
//         current = current->next;
//     }
// }

void page_fault(PCB_t *process){
    
    int index;

    if(load_page(process, process->pagenumber) == -1){      //if frame store is full then evict frame

        //TO ADD HERE: finding           
        char* evict = "Page fault! Victim page contents:\n";
        char buffer[1000];
        strcpy(buffer, evict);
        for(int i=0; i<3; i++){
            if(strcmp(mem_frame_get_line(index+i), "none")!=0){
                strcat(buffer, mem_frame_get_line(index+i));
                strcat(buffer, "\n");       // might need to add extra \ before newline
            }
        }
        strcat(buffer, "End of victim page contents.\n");
        printf("%s\n", buffer);
        mem_frame_delete(index);            //delete frame from store
        if(framestorepages[index/3]!=NULL){     //if process has not already been ended and destroyed, reflect frame deletion in its page table
            *framestorepages[index/3] = -1;
        }
        load_page(process, process->pagenumber);
    }
    
}

int run_process(char* policy){
    int errCode;
    int fault = 0;
    PCB_t *current = head;
    PCB_t *back = NULL;
    PCB_t *currentTemp = NULL;
    int k;
    if (strcmp(policy, "FCFS") == 0) { // first come first serve policy
        while ((head != NULL)) {
            while((head->counter)!=(head->length)){ // run all instructions of script
                if(head->pagetable[head->pagenumber] == -1){
                    page_fault(head);
                    current = head;
                    fault = 1;
                    if(head->next == NULL) break;            //if already at the end of the ready queue no rearranging needed
                    else{                    //if at head of queue
                        currentTemp = head;                   
                        currentTemp = currentTemp->next;
                        current->next = NULL;               
                        head = currentTemp;
                        while(currentTemp->next != NULL){       //send to back and move to next process
                            currentTemp = currentTemp->next;
                        }
                        currentTemp->next = current;
                        break;
                    }
                    
                }
                int index = head->pagetable[head->pagenumber] + head->counter % 3; // index is index pointed to by page table + offset from the counter
                errCode = parseInput(mem_frame_get_value(NULL, index));
                if (errCode != 0) { // exit upon error
                    end_all_process();
                    return errCode;
                }
                head->counter++;
                if (head->counter % 3 == 0) { // when counter is multiple of 3, increase page number to go to next page
                    head->pagenumber++;
                }
            }
            if(fault==0) end_process(head->pid); // end process when done
            fault = 0;
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
                if(current->pagetable[current->pagenumber] == -1){   //page fault
                    page_fault(current);
                    fault = 1;
                    if(current->next == NULL) break;            //if already at the end of the ready queue no rearranging needed
                    if(current == head){  
                        currentTemp = head;                   
                        currentTemp = currentTemp->next;
                        current->next = NULL;               
                        head = currentTemp;
                        while(currentTemp->next != NULL){       //send to back and move to next process
                            currentTemp = currentTemp->next;
                        }
                        currentTemp->next = current;
                        current = head->next;                   // go to next node
                        break;
                    }
                    else{                                       //if in middle of queue
                        currentTemp = head;
                        while(currentTemp->next != current){    //point node before current to one after current
                            currentTemp = currentTemp->next;
                        }
                        currentTemp->next = current->next;
                        back = currentTemp;
                        current->next = NULL;
                        while(back->next != NULL){              //send current to back and move to next process
                            back = back->next;
                        }    
                        back->next = current;
                        current = currentTemp->next;
                        break;
                    }
                    
                }
                int index = current->pagetable[current->pagenumber] + current->counter % 3; // index is index pointed to by page table + offset from the counter
                errCode = parseInput(mem_frame_get_value(NULL, index));
                if (errCode != 0) { // exit upon error
                    end_all_process();
                    return errCode;
                }
                current->counter++;
                if (current->counter % 3 == 0) {  // when counter is multiple of 3, increase page number to go to next page
                    current->pagenumber++;
                }
            }
            if(fault==0){
                if (current->next == NULL) current = head; // if at end of linked list, go back to head
                else current = current->next; // go to next node
            }
            fault = 0;
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
                if(minjob->pagetable[minjob->pagenumber] == -1){
                    page_fault(minjob);
                    fault = 1;
                    if(minjob->next == NULL) break;            //if already at the end of the ready queue no rearranging needed
                    if(minjob == head){  
                        currentTemp = head;                   
                        currentTemp = currentTemp->next;
                        minjob->next = NULL;               
                        head = currentTemp;
                        while(currentTemp->next != NULL){       //send to back and move to next process
                            currentTemp = currentTemp->next;
                        }
                        currentTemp->next = minjob;
                        minjob = head->next;                  // go to next node
                        break;
                    }
                    else{                                       //if in middle of queue
                        currentTemp = head;
                        while(currentTemp->next != minjob){    //point node before current to one after current
                            currentTemp = currentTemp->next;
                        }
                        currentTemp->next = minjob->next;
                        back = currentTemp;
                        minjob->next = NULL;
                        while(back->next != NULL){              //send current to back and move to next process
                            back = back->next;
                        }    
                        back->next = minjob;
                        break;
                    }
                }
                int index = minjob->pagetable[minjob->pagenumber] + minjob->counter % 3; // index is index pointed to by page table + offset from the counter
                errCode = parseInput(mem_frame_get_value(NULL, index));
                if (errCode != 0) { // exit upon error
                    end_all_process();
                    return errCode;
                }
                minjob->counter++;
                if (minjob->counter % 3 == 0) {  // when counter is multiple of 3, increase page number to go to next page
                    minjob->pagenumber++;
                }
            }
            if(fault==0) end_process(minjob->pid); // end process when done
            fault = 0;
        }
    }
    else if (strcmp(policy, "AGING") == 0) {        //TO DO: add page fault to aging
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
                if(head->pagetable[head->pagenumber] == -1){
                    //trigger page fault
                }
                int index = head->pagetable[head->pagenumber] + head->counter % 3; // index is index pointed to by page table + offset from the counter
                errCode = parseInput(mem_frame_get_value(NULL, index));
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

