#pragma once

/* - - - - - - - - */
/*    LIBRARIES    */
/* - - - - - - - - */

// standart libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>

// linux libs
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>

// remove later maybe
#include <sys/ipc.h>
#include <sys/shm.h>


/* - - - - - - - - - - - -*/
/*    TYPE DEFINITIONS    */
/* - - - - - - - - - - - -*/

/**
 * Constant macros
*/
#define KEY_MAX_SIZE 1000  // process's table (key max length
#define FILENAME "process_table.c" // file name for shared memory
#define IPC_RESULT_ERROR (-1) // error code for IPC functions

/**
 * States of a process
 */
typedef enum {
    // process has been terminated or is a zombie
    DEAD = 0,
    // process is sleeping
    SLEEPING = 1,
    // process is running
    RUNNING = 2,
} PTState;


/**
 * Definition of a process in process table, which is in a linked list of processes with the same key.
 */
typedef struct PTProcess {
    // process id
    pid_t pid;
    // process parent id
    pid_t ppid;
    // process state
    unsigned int state;
    // pointer pointing to next process, which has the same key (creates a linked list)
    struct PTProcess *nextProcess;
} *PTProcessPtr;

/**
 * Tag of a process, which is in a linked list of tags.
 */
typedef struct PTListTag {
    // the key/tag of a process
    char key[KEY_MAX_SIZE];
    // pointer to tag's data
    struct PTProcess *firstProcess;
    // pointer pointing to next tag
    struct PTListTag *nextTag;
} *PTListTagPtr;

/**
 * Definition of process_table/list, which is a 1 dimensional array of linked lists.
 * Uses hash function for quicker search and insertion of elements.
 */
typedef struct PTList {
    // dynamically alocated 1D array of PTProcess pointers
    struct PTListTag *firstTag;
    pid_t init_pid;
    // the size of the array
    size_t tag_num;
    // number of elements in the list
    size_t process_num;
} PTList;




/* - - - - - - - - - - - */
/*   PT_LIST FUNCTIONS   */
/* - - - - - - - - - - - */

/* Initiates the process table and returns pointer to it */
extern PTList* PT_Init(void); 

/* Cleares all memory and kills all the child processes */
//extern void PT_Dispose(PTList **list);    



/* - - - - - - - - - - - - - - - */
/*   PT_PROCESS BASIC FUNCTIONS  */
/* - - - - - - - - - - - - - - - */

/* Searches for element with key and returns pointer to it */
//extern PTProcessPtr PT_ProcessSearch(PTList *list, char *string);

/* Creates process in process table with no data */
extern void PT_ProcessCreate(PTList *list, char *tag);

/* Kills and deletes a process in the process table */
//extern void PT_ProcessDelete(PTList *list, char *string);   

/* Kills and deletes all processes in the process table */
//extern void PT_ProcessDeleteAll(PTList *list);   



/* - - - - - - - - - - - - - - - - */
/*   PT_PROCESS CONTROL FUNCTIONS  */
/* - - - - - - - - - - - - - - - - */
  
/**/



/* - - - - - - - - - - */
/*   DEBUG FUNCTIONS   */
/* - - - - - - - - - - */

/* (Debug function) - Prints the symbol table into stdout   */
extern void PT_PrintList(PTList *list);

/* (Debug function) - Prints the header of test */
extern void PT_PrintHeader(char *string);



/* - - - - - - - - - - - - - */
/*      OTHER FUNCTIONS      */
/* - - - - - - - - - - - - - */

/* Makes process sleep for n amount of miliseconds*/
int msec_sleep(long msec);

/* Gets useable shared memory block */
static int get_shmem_block(char *filename, int size);

/* Attaches shared memory block to processes*/
void* attach_shmem_block(char *filename, int size);

/* Detaches shared memory block from processes*/
bool detach_shmem_block(char *block);

/* Removes shared memory block */
bool remove_shmem_block(char *filename);

