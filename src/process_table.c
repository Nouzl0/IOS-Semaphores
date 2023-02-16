#include "process_table.h"

/* - - - - - - - - - - - - */
/*   ST_PROCESS FUNCTIONS  */
/* - - - - - - - - - - - - */

/**
 * Function creates (PTList), array of (linked lists), were every element in the linked list
 * corespondes to a process.
 * 
 * @param list Pointer to PTlist
 * @param size Number of (linked lists) which will be created.
 * @return Pointer to PTList if successful or NULL pointer if not
 */
extern PTList* PT_Init(size_t t_size, size_t p_size) 
{      
    // creating list struct
    PTList* list = mmap(NULL, sizeof(PTList), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_Init, mmap failed (PTList)\n");  
        return NULL;
    }

    // creating the list data
    list->semaphores = mmap(NULL, sizeof(struct PTListSemaphores), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (list->semaphores == NULL) {
        fprintf(stderr, "ERROR - PT_Init, mmap failed (PTProcessData)\n");  
        return NULL;
    }

    // creating array tags
    list->t_arr = mmap(NULL, t_size * sizeof(struct PTListTag), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (list->t_arr == NULL) {
        fprintf(stderr, "ERROR - PT_Init, mmap failed (PTListTag)\n");  
        return NULL;
    }

    // creating process nodes
    for (int i = 0; i < t_size; i++) {
        list->t_arr[i].p_arr = mmap(NULL, p_size * sizeof(struct PTProcess), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (list->t_arr[i].p_arr == NULL) {
            fprintf(stderr, "ERROR - PT_Init, mmap failed (PTProcess)\n");  
            return NULL;
        }
    }

    // opening semaphores
    list->semaphores->synchronize_all.sem = sem_open("/synchronize_all", O_CREAT, 0644, 0);
    list->semaphores->synchronize_all.state = SEM_NOT_INIT;
    list->semaphores->synchronize_all.running_p = 0;
    list->semaphores->synchronize_all.sleeping_p = 0;

    list->semaphores->synchronize_tag_one_a.sem = sem_open("/synchronize_tag_one_a", O_CREAT, 0644, 0);
    list->semaphores->synchronize_tag_one_a.state = SEM_NOT_INIT;
    list->semaphores->synchronize_tag_one_a.running_p = 0;
    list->semaphores->synchronize_tag_one_a.sleeping_p = 0;
    
    list->semaphores->synchronize_tag_one_b.sem = sem_open("/synchronize_tag_one_b", O_CREAT, 0644, 0);
    list->semaphores->synchronize_tag_one_b.state = SEM_NOT_INIT;
    list->semaphores->synchronize_tag_one_b.running_p = 0;
    list->semaphores->synchronize_tag_one_b.sleeping_p = 0;

    list->semaphores->synchronize_tag_one_c.sem = sem_open("/synchronize_tag_one_c", O_CREAT, 0644, 0);
    list->semaphores->synchronize_tag_one_c.state = SEM_NOT_INIT;
    list->semaphores->synchronize_tag_one_c.running_p = 0;
    list->semaphores->synchronize_tag_one_c.sleeping_p = 0;

    // adding data of the process table
    list->p_size = p_size;
    list->t_size = t_size;
    list->t_num = 0;

    // adding data of the intialization process
    list->init_pid.pid = getpid();
    list->init_pid.ppid = getppid();
    list->init_pid.state = RUNNING;

    return list;
}

/**
 * Deletes process table and kill all the processes.
 * - uses function [ PT_DeleteAll ].
 * 
 * @param list Pointer to PTlist, which will be deallocated
 */
//void PT_Dispose(STList **list)
//{
//    // checking if list is empty
//    if ((*list) == NULL) {
//        fprintf(stderr ,"ERROR - ST_Dispose, symtable is already empty\n");
//        return;
//    }
//
//    // deleting symtable
//    ST_DeleteAll(*list);
//    free((*list)->array);
//    (*list)->array = NULL;
//
//    // setting list pointer to NULL
//    free(*list);
//    *list = NULL;
//}



/* - - - - - - - - - - - - */
/*   ST_ELEMENT FUNCTIONS  */
/* - - - - - - - - - - - - */

/**
 * Function searches (PT_Process) in the symtable and returns pointer to it
 */
extern void PT_ProcessSearch(PTList *list, pid_t in_pid, PTListTagPtr ret_tag, PTProcessPtr ret_process) 
{
    // if list is empty
    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_ProcessSearch, list is empty\n");
        ret_tag = NULL;
        ret_process = NULL;
        return;
    }

    // searching the process in the list
    for (int i = 0; i < list->t_size; i++) {
        for (int j = 0; j < list->p_size; j++) {
            if (list->t_arr[i].p_arr[j].pid == in_pid) {
                ret_tag = &(list->t_arr[i]);
                ret_process = &(list->t_arr[i].p_arr[j]);
                return;
            }
        }
    }
    
    // returning NULL if process is not found
    ret_tag = NULL;
    ret_process = NULL;

}


/**
 * Creates a process in process table with no data
 */
extern void PT_ProcessCreate(PTList *list, char *tag)
{   
    // checking if list is empty
    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_ProcessCreate, list is empty\n");
        return;
    }

    // checking if process is the list proceess
    if (getpid() != list->init_pid.pid) {
        return;
    }

    // pointer data
    PTListTagPtr tag_ptr = NULL;
    PTProcessPtr process_ptr = NULL;
    bool dead_process = false;

    // searching for tag
    for (int i = 0; i < list->t_size; i++) {
        if ((strcmp(list->t_arr[i].key, tag) == 0)) {
            tag_ptr = &(list->t_arr[i]);
            break;
        }
    }

    // if tag doesn't exist's, create it for new process
    if (tag_ptr == NULL) {
        
        // list doesn't have any space for new tags
        if (list->t_num == list->t_size) {
            fprintf(stderr, "ERROR - PT_ProcessCreate, there is no free space for new tags (list is full)\n");
            return;
        }

        // add data to new tag
        tag_ptr = &(list->t_arr[list->t_num]);
        list->t_num++;
        strcpy(tag_ptr->key, tag);
        process_ptr = &(tag_ptr->p_arr[0]);
        
    // if tag exist's, add it to existing tag
    } else {
        // rewriting existing tag
        if (tag_ptr->p_num == 0) {
            strcpy(tag_ptr->key, tag);
            process_ptr = &(tag_ptr->p_arr[0]);
        
        // searching for free space in tag
        } else {

            // searching for free space in tag
            for (int i = 0; i < tag_ptr->p_num; i++) {
                if (tag_ptr->p_arr[i].state == 0) {
                    process_ptr = &(tag_ptr->p_arr[i]);
                    dead_process = true;
                }
            }

            // if there is no free space in tag, create new process
            if (dead_process == false) {
                if (tag_ptr->p_num == list->p_size) {
                    fprintf(stderr, "ERROR - PT_ProcessCreate, there is no free space for processes (tag is full)\n");
                    return;
                } else {
                    process_ptr = &(tag_ptr->p_arr[tag_ptr->p_num]);
                }
            }
        }
    }

    // creating new process and adding the apropiate data
    tag_ptr->p_num++;
    tag_ptr->p_run++;
    process_ptr->state = RUNNING;

    pid_t pid = fork();

    if (pid == 0) {
        process_ptr->pid = getpid();
        process_ptr->ppid = getppid();
    }
}


/**
 * Deletes an element in symbol 
 */
//extern void PT_ProcessDelete(PTList *list, char *string )
//{
//}


/**
 * Deletes every element in the symbol table
 */
//extern void PT_DeleteAll(PTList *list) 
//{   
//}


/* - - - - - - - - - - - */
/*   CONTORL FUNCTIONS   */
/* - - - - - - - - - - - */


/**
 * Synchronises all processes in the list
 */
extern void PT_SynchroniseAll(PTList *list)
{
    // checking if list is empty
    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_SynchroniseAll, list is empty\n");
        return;
    }

    // initialising semaphore
    bool sem_init_process = false;

    if (list->semaphores->synchronize_all.state == SEM_NOT_INIT) {
        list->semaphores->synchronize_all.state = SEM_IN_INIT;
        sem_init_process = true;
    }

    // initialising semaphore
    if (list->semaphores->synchronize_all.state == SEM_IN_INIT) {

        // count the active number of processes
        unsigned int active_p = 0;

        for (int i = 0; i < list->t_num; i++) {
            for (int j = 0; j < list->t_arr[i].p_num; j++) {
                if (list->t_arr[i].p_arr[j].state != DEAD) {
                    active_p++;
                }
            }
        }

        // marking the active processes
        if (sem_init_process == true) {
            list->semaphores->synchronize_all.running_p = active_p;
            list->semaphores->synchronize_all.state = SEM_INIT;
            //printf("p_set_up = %d, active_p = %d, sleep_p = %d\n", getpid(), list->semaphores->synchronize_all.running_p, list->semaphores->synchronize_all.sleeping_p);
            //fflush(stdout);
        }
    }



    // stopping the process if it is not the last one active
    if (((list->semaphores->synchronize_all.running_p - 1) <= list->semaphores->synchronize_all.sleeping_p) && (list->semaphores->synchronize_all.running_p > 0)) {
        
        // saving the number of sleeping processes
        unsigned int sleeping_p = list->semaphores->synchronize_all.sleeping_p;
        //printf("\np_wake = %d, active_p = %d, sleep_p = %d\n", getpid(), list->semaphores->synchronize_all.running_p, sleeping_p);
        //fflush(stdout);

        // resetting the semaphore
        list->semaphores->synchronize_all.state == SEM_NOT_INIT;
        list->semaphores->synchronize_all.running_p = 0;
        list->semaphores->synchronize_all.sleeping_p = 0;

        // waking up all the processes
        for (int i = 0; i < sleeping_p; i++) {
            sem_post(list->semaphores->synchronize_all.sem);
        }

    // waking up all the processes
    } else {
        //printf("p_sleep = %d, active_p = %d, sleep_p = %d\n", getpid(), list->semaphores->synchronize_all.running_p, list->semaphores->synchronize_all.sleeping_p);
        //fflush(stdout);
        list->semaphores->synchronize_all.sleeping_p++;
        sem_wait(list->semaphores->synchronize_all.sem);
    }
}

/**
 * Synchronizes every and only one process from a tag in 
 */
extern void PT_SynchroniseTagOne(PTList *list) 
{
    // checking if list is empty
    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_SynchroniseAll, list is empty\n");
        return;
    }

    // initialising semaphore
    bool sem_init_process = false;

    if (list->semaphores->synchronize_tag_one_a.state == SEM_NOT_INIT) {
        list->semaphores->synchronize_tag_one_a = SEM_IN_INIT;
        sem_init_process = true;
    }

    // initialising semaphore
    if (list->semaphores->synchronize_all.state == SEM_IN_INIT) {

        // count the active number of processes
        unsigned int active_p_a = 0;
        unsigned int active_p_b = 0;

        for (int i = 0; i < list->t_num; i++) {
            for (int j = 0; j < list->t_arr[i].p_num; j++) {
                if (list->t_arr[i].p_arr[j].state != DEAD) {
                    switch (j) {
                    case 0:
                        active_p_a++;
                        break;
                    case 1:
                        active_p_b++;
                        break;
                    default: break;
                    }
                }
            }
        }

        // marking the active processes
        if (sem_init_process == true) {
            list->semaphores->synchronize_tag_one_a = active_p_a;
            list->semaphores->synchronize_tag_one_b = active_p_b;
            list->semaphores->synchronize_tag_one_a = SEM_INIT;
            //printf("p_set_up = %d, active_p = %d, sleep_p = %d\n", getpid(), list->semaphores->synchronize_all.running_p, list->semaphores->synchronize_all.sleeping_p);
            //fflush(stdout);
        }
    }

    // searching for the processes tag
    int tag = -1;
    PT_ProcessSearch(list, getpid(), 0);


    // stopping the process if it is not the last one active
    if ((list->semaphores->synchronize_tag_one_a.running_p > 0) && (list. > 0)) {
        
        // saving the number of sleeping processes
        unsigned int sleeping_p = list->semaphores->synchronize_all.sleeping_p;
        //printf("\np_wake = %d, active_p = %d, sleep_p = %d\n", getpid(), list->semaphores->synchronize_all.running_p, sleeping_p);
        //fflush(stdout);

        // resetting the semaphore
        list->semaphores->synchronize_tag_one_a.sleeping_p = 0;

        // waking up all the processes
        sem_post(list->semaphores->synchronize_tag_one_a);
        sem_post(list->semaphores->synchronize_tag_one_b);
        sem_post(list->semaphores->synchronize_tag_one_c);

    // waking up all the processes
    } else {
        //printf("p_sleep = %d, active_p = %d, sleep_p = %d\n", getpid(), list->semaphores->synchronize_all.running_p, list->semaphores->synchronize_all.sleeping_p);
        //fflush(stdout);
        list->semaphores->synchronize_all.sleeping_p++;
        sem_wait(list->semaphores->synchronize_all.sem);
    }


}



/* - - - - - - - - - - */
/*   DEBUG FUNCTIONS   */
/* - - - - - - - - - - */

/**
 * Prints the list (debug)
 */
extern void PT_PrintList(PTList *list)
{   
    if (list->init_pid.pid != getpid()) {
        return;
    }


    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_PrintList, list is empty\n");
        return;
    }

    // printing list data
    printf(" t_size: %ld, p_size: %ld, init_pid: %d\n", list->t_size, list->p_size, list->init_pid.pid);

    // going through every tag
    for (int i = 0; i < list->t_num; i++) {

        // printing tag
        printf("( %s ) ->", list->t_arr[i].key);

        // going through every process in tag
        for (int j = 0; j < list->t_arr[i].p_num; j++) {
            printf("[ %d, %d, %d ]; ", list->t_arr[i].p_arr[j].pid, 
            list->t_arr[i].p_arr[j].ppid, list->t_arr[i].p_arr[j].state);
        }
        printf("\n");
    }
    printf(" - - - - - - -\n");
}



/* - - - - - - - - - - */
/*   OTHER FUNCTIONS   */
/* - - - - - - - - - - */

/* Makes process sleep for n amount of miliseconds*/
int msec_sleep(long msec)
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

int ran_msec_sleep(long max_msec)
{
    // initialising random seed
    static bool init = false;
    if (init == 0) {
        srand(getpid());
        init = true;
    }

    // generating random number
    long msec = (rand() % max_msec);

    // sleeping for random amount of miliseconds
    return msec_sleep(msec);
}