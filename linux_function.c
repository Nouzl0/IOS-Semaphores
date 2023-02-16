#include <stdio.h>

//standart libraries
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>          // used in msleep
#include <time.h>           // used in msleep

#include <sys/ipc.h>
#include <sys/shm.h>

// linux libraries          
#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */
#include <fcntl.h>          /* O_CREAT, O_EXEC          */
#include <signal.h>         /* kill(), */
#include <sys/mman.h>       /* mmap */
#include <unistd.h>         /* fork(), */    

#include "linux_functions.h"


/**
 *  - DESCRIPTION
 *  Function puts proces to sleep for n-amount of miliseconds. 
 *  The sleep will continue if it is interupted by signal.
 * 
 *  - INPUT 
 *  takes (int msec), number of miliseconds a process will sleep
 * 
 * - OUTPUT
 *  returns (-1) if msleep fails
 * 
 *  - USES LIBRARIES  
 *  time.h, errno.h 
 * 
 * - CREATORS 
 *  function partially copied from - https://stackoverflow.com/a/1157217 
 *  creator of the post - https://stackoverflow.com/users/134633/caf
 *  post edited by - https://stackoverflow.com/users/4298200/neuron-freedom-for-ukraine
**/

int msleep(long msec)
{
    struct timespec ts;
    int res = 0;

    // checking errors
    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    // setting nanosleep values to milisecond
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    // nanosleep executes
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}


/** 
 *  * INITIALIZE NEW PARENT PROCESS - MACRO
 * 
**/
PROCESS initialize_parent(int type) 
{
    PROCESS parent_p;

    // setting up data   
    parent_p.pid = getpid();
    parent_p.state = RUNNING;
    parent_p.type = type;

    return parent_p;
}


/** 
 *  * CREATES PROCESSES FROM ONE PARENT
 * 
 *  - creates num number of children processes from parent process
 * 
 *  - litst all processes into dynamic array of PROCESS which is shared 
 *    between children processes and parent process
 * 
 *  - to destroy all processes it is required to use munmap as well
**/
FAMILY_P create_child_processes(PROCESS parent_p, int num_of_children)
{   
    // declaring group of processes
    FAMILY_P group_p;


    // creating children processes
    if (getpid() == parent_p.pid) { 

         // declaring shared data
        group_p.parent_p = parent_p;
        group_p.size = num_of_children;
        group_p.child_p = mmap(NULL , num_of_children * sizeof(PROCESS), 
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0); 
        
        // temp. variable for pid data
        pid_t *group_pid = malloc(num_of_children * sizeof(pid_t));

        // creating child processes and getting their pid_t
        for (int i = 0; i < num_of_children; i++) {
            
            // if it is child process it jumps to end
            if ((group_pid[i] = fork()) == 0) { 
                goto jump_to_end;
            }
        }

        // writing data about children which are shared between them
        for (int i = 0; i < num_of_children; i++) {
            group_p.child_p[i].pid = group_pid[i];
            group_p.child_p[i].type = parent_p.type;
            group_p.child_p[i].state = RUNNING;
        }
            
        // function end for children & parent
        free(group_pid);

        jump_to_end:
        return group_p;
    } 
    
    // functoin end for every other program
    group_p.size = 0;
    return group_p;
}


/** 
 *  * DESTROYS PROCESSES OF ONE PARENT
 * 
 *  - destroys processes and cleares memory of child_p
 * 
**/
void destroy_child_processes(FAMILY_P group_p)
{
    if (getpid() == group_p.parent_p.pid) {

        for (int i = 0; i < group_p.size; i++)
        {
            if (group_p.child_p[i].state != ZOMBIFIED) {
                kill(group_p.child_p[i].pid, SIGKILL);
            }
        }

        munmap(group_p.child_p, group_p.size * sizeof(PROCESS));
    }
}


/** 
 *  * PRINTS PROCESSES OF ONE PARENT
**/
void print_family_processes(FAMILY_P group_p)
{
    if (getpid() == group_p.parent_p.pid) {
        
        printf("\n[PARENT] -  pid = %d,  state = %d,  type = %d\n", 
          group_p.parent_p.pid, group_p.parent_p.state, group_p.parent_p.type);
        
        printf(" -\n");
        
        for (int i = 0; i < group_p.size; i++) {
            printf("[%d] -  pid = %d,  state = %d,  type = %d\n", i,
              group_p.child_p[i].pid, group_p.child_p[i].state, group_p.child_p[i].type);
        }
    }
}


/** 
 *  * PRINTS PROCESSES OF ONE PARENT - WILL PROBABLY BE MACRO
**/
void print_active_processes(FAMILY_P group_p) {

    if (is_child_process(group_p)) {
        printf(" - pid = %d,  ppid = %d\n", getpid(), getppid());
    }

}



size_t asleep_child_processes(FAMILY_P group_p) {

    size_t num = 0;

    for (int i = 0; i < group_p.size; i++) {
        if (group_p.child_p[i].state == SLEEPING) {
            num++;
        }
    }

    return num;
}

//void wait_for_all(FAMILY_P group_p) {
//
//    // would be shared data
//    int shmget(key_t key, size_t size, int shmflg)
//    int num = 0;
//
//    // shared data function
//    if(is_child_process(group_p)) {
//        printf("%d - %d \n",getpid() ,num++);
//    }
//}
//