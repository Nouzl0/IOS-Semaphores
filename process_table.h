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

// linux semaphore libs
#include <semaphore.h>
#include <sys/ipc.h>
#include <fcntl.h> 

// remove later maybe
#include <sys/shm.h>


/* - - - - - - - - - - - -*/
/*    TYPE DEFINITIONS    */
/* - - - - - - - - - - - -*/

/**
 * Constant macros
*/
#define KEY_MAX_SIZE 1000  // process's table (key max length

/**
 *  Macro functions
 */
#define is_init_pid(list) (list->init_pid.pid == getpid())



/* - - - - - - - - - - - */
/*         ENUMS         */
/* - - - - - - - - - - - */

/* States of a process */
typedef enum {
    // process has been terminated or is a zombie
    DEAD = 0,
    // process is sleeping
    SLEEPING = 1,
    // process is running
    RUNNING = 2,
} PTProcessState;

/*States of a semaphore */
typedef enum {
    // semaphore is not initialized
    SEM_NOT_INIT = 0,
    // semaphore is being initialized
    SEM_IN_INIT = 1,
    // semaphore is initialized and being used
    SEM_INIT = 2,
} PTSemaphoreState;








/* - - - - - - - - - - - - */
/*   PT_LIST SHARED DATA   */
/* - - - - - - - - - - - - */

/* Shared data of a process in process table */
typedef struct SM_Counter {
    unsigned int data;
    PTSemaphoreState sem_state; 
    sem_t sem_1;
    int sem_wait_count;
} SM_Counter;

typedef struct SM_Wait{
    sem_t sem;
    PTSemaphoreState sem_state;
    pid_t waiting_pid;
    int processes_passed;
    int process_count;
} SM_Wait;

/* Definition of semaphores in process table */
//typedef struct PTListSemaphores {
//    // semaphore for synchronize_all function
//    struct PTSemaphore synchronize_all;
//    // semaphores for synchronize_tag_one function
//    struct PTSemaphore synchronize_tag_one_a;
//    struct PTSemaphore synchronize_tag_one_b;
//    struct PTSemaphore synchronize_tag_one_c;
//
//} *PTListSemaphoresPtr;



/* - - - - - - - - - - - */
/*    PT_LIST DATA       */
/* - - - - - - - - - - - */

/* Definition of a process in process table, which is in a linked list of processes with the same key. */
typedef struct PTProcess {
    // process id
    pid_t pid;
    // process parent id
    pid_t ppid;
    // process state
    unsigned int state;
} *PTProcessPtr;

/* Definition of semaphore */
typedef struct PTSemaphore {
    // semaphore controll
    sem_t *sem;
    // semaphore state
    unsigned int state;
    // number of sleeping processes
    unsigned int sleeping_p;
    // number of running processes
    unsigned int running_p;
} *PTSemaphore;

/* Tag of a process, which is in a linked list of tags. */
typedef struct PTListTag {
    // the key/tag of a process
    char key[KEY_MAX_SIZE];
    // pointer to tag's data
    struct PTProcess *p_arr;
    // number of processes in the tag
    unsigned int p_num;
    // number of sleeping processes
    unsigned int p_sleep;
    // number of running processes
    unsigned int p_run;
} *PTListTagPtr;

/* Shared data of a process in process table -> added by user */
typedef struct PTListData {
    struct SM_Counter cnt;             // basic counter used by multiple processes
    struct SM_Wait wait;               // basic wait used by multiple processes
} *PTListDataPtr;

/**
 * Definition of process_table/list, which is a 1 dimensional array of arrays which arrays.
 * Uses hash function for quicker search and insertion of elements.
 */
typedef struct PTList {
    
    // dynamically alocated 1D array of PTProcess pointers
    struct PTListTag *t_arr;
    // process shared data
    struct PTListData *shared_data;
    
    // max number of tags in the list
    size_t t_size;
    // max number of processes in one tag
    size_t p_size;
    // number of tags in the list
    unsigned int t_num;

    // process which initialized the process table
    struct PTProcess init_pid;
    
} PTList;



/* - - - - - - - - - - - */
/*   PT_LIST FUNCTIONS   */
/* - - - - - - - - - - - */

/* Initiates the process table and returns pointer to it */
extern PTList* PT_Init(size_t t_size, size_t p_size); 

/* Cleares all memory and kills all the child processes */
//extern void PT_Dispose(PTList **list);    



/* - - - - - - - - - - - - - - - */
/*   PT_PROCESS BASIC FUNCTIONS  */
/* - - - - - - - - - - - - - - - */

/* Searches for process with key and returns pointer to it */
extern void PT_ProcessSearch(PTList *list, pid_t in_pid, PTListTagPtr ret_tag, PTProcessPtr ret_process);

/* Creates process in process table with no data */
extern void PT_ProcessCreate(PTList *list, char *tag);

/* Kills and deletes a process in the process table */
//extern void PT_ProcessDelete(PTList *list, char *string);   

/* Kills and deletes all processes in the process table */
//extern void PT_ProcessDeleteAll(PTList *list);   



/* - - - - - - - - - - - - - - - - */
/*      SHARED DATA FUNCTIONS      */
/* - - - - - - - - - - - - - - - - */
  
/* initialize counter */
int SM_CounterInit(PTListDataPtr shared_data);

/* print counter-data and increments it */
int SM_CounterPrint(PTListDataPtr shared_data, char *message);

/* destroy semaphores in counter*/
int SM_CounterDestroy(PTListDataPtr shared_data);


/* initialize wait semaphores*/
int SM_WaitInit(PTListDataPtr shared_data, pid_t waiting_process_id ,int process_count);

/* process waits for all processes to end*/
int SM_WaitForAll(PTListDataPtr shared_data);

/* Synchronizes all the processes */
int SM_WaitDestroy(PTListDataPtr shared_data);


/* - - - - - - - - - - */
/*   DEBUG FUNCTIONS   */
/* - - - - - - - - - - */

/* (Debug function) - Prints the symbol table into stdout   */
extern void PT_PrintList(PTList *list);



/* - - - - - - - - - - - - - */
/*      OTHER FUNCTIONS      */
/* - - - - - - - - - - - - - */

/* Makes process sleep for n amount of miliseconds */
int msec_sleep(long msec);

/* Makes process sleep for random amount of time */
int ran_msec_sleep(int min_msec, int max_msec);
