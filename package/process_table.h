/** @file process_table.h
 *  @author Nikolas Nosál (xnosal01@stud.fit.vutbr.cz)
 *  @date 2023-04-24
 */

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
#include <signal.h>

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
    
    /* synchronisation data*/
    sem_t sem;
    PTSemaphoreState sem_state;
    pid_t waiting_pid;
    int processes_passed;
    int process_count;
    /* mutex lock data*/
    sem_t mutex;
} SM_Wait;


typedef struct SM_Office {
    int is_open; 
    
    // service 1
    sem_t sem_1;
    int sem_1_count;
    unsigned int timeout_1;

    // service 2
    sem_t sem_2;
    int sem_2_count;
    unsigned int timeout_2;

    // service 3
    sem_t sem_3;
    int sem_3_count;
    unsigned int timeout_3;
} SM_Office;


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
    struct SM_Office office;           // office data needed for the given task (office)
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
extern int PT_Destroy(PTList **list); 


/* - - - - - - - - - - - - - - - */
/*   PT_PROCESS BASIC FUNCTIONS  */
/* - - - - - - - - - - - - - - - */

/* Seaches for process and returns it's tag and it's position in the 2d array */
extern int PT_ProcessSearch(PTList *list, pid_t pid, char *tag, int *tag_num, int *pro_num);

/* Creates process in process table with no data */
extern void PT_ProcessCreate(PTList *list, char *tag);

/* Checks if process is in the given tag */
extern int PT_IsTag(PTList *list, char *tag);


/* - - - - - - - - - - - - - - - - - - */
/*         SM_COUNTER FUNCTIONS        */
/* - - - - - - - - - - - - - - - - - - */
  
/* initialize counter */
int SM_CounterInit(PTListDataPtr shared_data);

/* print counter-data and increments it */
int SM_CounterPrint(PTListDataPtr shared_data, FILE *file, char *message);

/* destroy semaphores in counter*/
int SM_CounterDestroy(PTListDataPtr shared_data);

 
/* - - - - - - - - - - - - - - - - - - */
/*          SM_OFFICE FUNCTIONS        */
/* - - - - - - - - - - - - - - - - - - */

/* initialize office data */
int SM_OfficeInit(PTListDataPtr shared_data);

/* destroy office data */
int SM_OfficeDestroy(PTListDataPtr shared_data);

/* set office to close-state*/
void SM_OfficeClose(PTListDataPtr shared_data);

/* officer takes a break -> used by serve*/
int SM_OfficeBreak(PTListDataPtr shared_data, FILE *log_file, int process_id, unsigned int max_break_time);

/* officer servers a servis */
int SM_OfficeServe(PTListDataPtr shared_data, FILE *log_file, int process_id, unsigned int max_break_time);

/* customer gets service he desires*/
int SM_OfficeService(PTListDataPtr shared_data, FILE *log_file, int process_id, int type_of_service);


/* - - - - - - - - - - - - - - - - - */
/*          SM_WAIT FUNCTIONS        */
/* - - - - - - - - - - - - - - - - - */

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
/*      SLEEP FUNCTIONS      */
/* - - - - - - - - - - - - - */

/* Makes process sleep for n amount of miliseconds */
int msec_sleep(long msec);

/* Makes process sleep for random amount of time */
int ran_msec_sleep(int min_msec, int max_msec);
