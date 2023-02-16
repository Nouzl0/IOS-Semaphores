#pragma once

// linux libraries          
#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */
#include <fcntl.h>          /* O_CREAT, O_EXEC          */
#include <signal.h>         /* kill(), */
#include <sys/mman.h>       /* mmap */
#include <unistd.h>         /* fork(), */    
#include <stdbool.h>


/**
 * *TYPE DECLARETIONS
**/

// process def. type
typedef struct process
{
    pid_t pid;  // process id
    int state;  // state of a process 
    int type;   // type of process, defined by the user
} PROCESS;

// state of a process
typedef enum pid_state 
{
    ZOMBIFIED = 0,
    RUNNING = 1,
    SLEEPING = 2,
    SUSPENDED = 3,
    RUNNABLE = 4
} PID_STATE;

// process def. type
typedef struct family_p
{   
    PROCESS parent_p;    // parent_process identification
    PROCESS *child_p;    // all children_processes
    int size;            // number of children processes in a faimily
} FAMILY_P;


/**
 *  *MACROS
 *  TODO - get more macros
**/

//
#define is_process(process_p)       ((getpid() == process_p.group.pid))
#define is_parent_process(group_p)  ((getpid() == group_p.parent_p.pid))
#define is_child_process(group_p)   ((getpid() >= group_p.child_p[0].pid) && (getpid() <= group_p.child_p[group_p.size - 1].pid))

/**
 *  *PROCESS FUNCTIONS
 *  
 *  TODO - Create variable for process
**/

// sleep
//int msleep(long msec);  // puts procces to sleep 

// process function
PROCESS initialize_parent(int type);
FAMILY_P create_child_processes(PROCESS parent_p, int num_of_children);
void destroy_child_processes(FAMILY_P group_p);
void print_family_processes(FAMILY_P group_p);
void print_active_processes(FAMILY_P group_p);

// semaphore functions
//void wait_for_all(FAMILY_P group_p);

// old functions
//void wait_for_parent(pid_t parent_p, int child_p_num);  // children processes wait for parent
//void wait_for_children(pid_t parent_p, int child_p_num); //parent process waits for children waits 