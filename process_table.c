#include "process_table.h"

/* - - - - - - - - - - - - */
/*   ST_PROCESS FUNCTIONS  */
/* - - - - - - - - - - - - */

/**
 * Function creates (PTList), array of (linked lists), were every element in the linked list
 * corespondes to a process.
 * 
 * @param list Pointer to PTlist
 * @param t_size Number of array which will be created
 * @param p_size Number of processes in the array
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
    list->shared_data = mmap(NULL, sizeof(struct PTListData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_Init, mmap failed (PTListDat)\n");  
        return NULL;
    }

    // creating array tags
    list->t_arr = mmap(NULL, t_size * sizeof(struct PTListTag), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (list->t_arr == NULL) {
        fprintf(stderr, "ERROR - PT_Init, mmap failed (PTListTag)\n");  
        return NULL;
    }

    // creating process nodes
    for (unsigned int i = 0; i < t_size; i++) {
        list->t_arr[i].p_arr = mmap(NULL, p_size * sizeof(struct PTProcess), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (list->t_arr[i].p_arr == NULL) {
            fprintf(stderr, "ERROR - PT_Init, mmap failed (PTProcess)\n");  
            return NULL;
        }
    }

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
//extern void PT_ProcessSearch(PTList *list, pid_t in_pid, PTListTagPtr ret_tag, PTProcessPtr ret_process) 
//{
//    // if list is empty
//    if (list == NULL) {
//        fprintf(stderr, "ERROR - PT_ProcessSearch, list is empty\n");
//        ret_tag = NULL;
//        ret_process = NULL;
//        return;
//    }
//
//    // searching the process in the list
//    for (unsigned int i = 0; i < list->t_size; i++) {
//        for (unsigned int j = 0; j < list->p_size; j++) {
//            if (list->t_arr[i].p_arr[j].pid == in_pid) {
//                ret_tag = &(list->t_arr[i]);
//                ret_process = &(list->t_arr[i].p_arr[j]);
//                return;
//            }
//        }
//    }
//    
//    // returning NULL if process is not found
//    ret_tag = NULL;
//    ret_process = NULL;
//}


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
    for (unsigned int i = 0; i < list->t_size; i++) {
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
            for (unsigned int i = 0; i < tag_ptr->p_num; i++) {
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
/*       SM_COUNTER      */
/* - - - - - - - - - - - */

/* initialize counter */
int SM_CounterInit(PTListDataPtr shared_data)
{
    // semaphore is already initialised 
    if (shared_data->cnt.sem_state == SEM_IN_INIT || shared_data->cnt.sem_state == SEM_INIT) {
        fprintf(stderr, "ERROR - SM_CounterInit, semaphore is already initialised\n");
        return -1;
    }
    
    // initialising counter data
    shared_data->cnt.sem_state = SEM_INIT;
    shared_data->cnt.data = 0;
    shared_data->cnt.sem_wait_count = 0;

    // initialising semaphore 1
    if (sem_init(&(shared_data->cnt.sem_1), 0, 1) == -1) {
        fprintf(stderr, "ERROR - SM_CounterInit, sem_init failed\n");
        return -1;
    }

    return 0;
}

/* print counter-data and increment */
int SM_CounterPrint(PTListDataPtr shared_data, char *message)
{
    // [0] - SEM-WAIT
    // lost processes are availabled by this data-struct
    if (shared_data->cnt.sem_wait_count == 2) {
        sem_post(&(shared_data->cnt.sem_1));
        ran_msec_sleep(50, 150);
    }

    // increasing number of sleeping processes
    shared_data->cnt.sem_wait_count++;

    if (sem_wait(&(shared_data->cnt.sem_1)) == -1) {
        fprintf(stderr, "ERROR\n");
    }

    // [1] - PRINT
    // print message and increment counter
    printf("%d: %s\n", shared_data->cnt.data++, message);


    // [2] - SEMPOST

    // decresing the number of processes
    shared_data->cnt.sem_wait_count--;

    if (sem_post(&(shared_data->cnt.sem_1)) == -1) {
        fprintf(stderr, "ERROR - SM_CounterPrint, sem_post failed\n");
        return -1;
    }

    return 0;
}

/* destroy counter-data */
int SM_CounterDestroy(PTListDataPtr shared_data)
{
    // destroy semaphore
    if (sem_destroy(&(shared_data->cnt.sem_1)) == -1) {
        fprintf(stderr, "ERROR - SM_CounterDestroy, sem_destroy failed\n");
        return -1;
    }

    // unset counter data
    shared_data->cnt.sem_state = SEM_NOT_INIT;
    shared_data->cnt.data = 0;

    return 0;
}


/* - - - - - - - - - - - */
/*        SM_WAIT        */
/* - - - - - - - - - - - */

/* initialize wait semaphores*/
int SM_WaitInit(PTListDataPtr shared_data, pid_t waiting_process_id ,int process_count)
{
    // semaphore is already initialised
    if (shared_data->wait.sem_state == SEM_IN_INIT || shared_data->wait.sem_state == SEM_INIT) {
        fprintf(stderr, "ERROR - SM_WaitInit, semaphore is already initialised\n");
        return -1;
    }

    // initialising semaphore
    if (sem_init(&(shared_data->wait.sem), 0, 0) == -1) {
        fprintf(stderr, "ERROR - SM_WaitInit, sem_init failed\n");
        return -1;
    }

    // initialising wait data
    shared_data->wait.sem_state = SEM_INIT;
    shared_data->wait.waiting_pid = waiting_process_id;
    shared_data->wait.process_count = process_count;
    shared_data->wait.processes_passed = 0;

    return 0;
}

/* process waits for all processes to end*/
int SM_WaitForAll(PTListDataPtr shared_data)
{   
    // check if data is initialised
    if (shared_data->wait.sem_state != SEM_INIT) {
        fprintf(stderr, "ERROR - SM_WaitForAll, semaphore is not initialised\n");
        return -1;
    }

    // sleep process with the given id
    if ((getpid() == shared_data->wait.waiting_pid) && (shared_data->wait.processes_passed < shared_data->wait.process_count)) {
        
        // sleep the target process
        if (sem_wait(&(shared_data->wait.sem)) == -1) {
            fprintf(stderr, "ERROR - SM_WaitForAll, sem_wait failed\n");
            return -1;
        }
    
        // sleep the process for short time after waking up
        msec_sleep(10);

    // increment number of processes that passed the semaphore
    } else {
        
        // if procceses passed is equal to the number of processes free the semaphore
        if (shared_data->wait.processes_passed == shared_data->wait.process_count) {    
            if (sem_post(&(shared_data->wait.sem)) == -1) {
                fprintf(stderr, "ERROR - SM_WaitForAll, sem_post failed\n");
                return -1;
            }
        
        // increment number of processes that passed the semaphore
        } else {
            shared_data->wait.processes_passed++;
        }
    }

    return 0;
}

/* Synchronizes all the processes */
int SM_WaitDestroy(PTListDataPtr shared_data)
{
    // destroy semaphore
    if (sem_destroy(&(shared_data->wait.sem)) == -1) {
        fprintf(stderr, "ERROR - PT_WaitDestroy, sem_destroy failed\n");
        return -1;
    }

    // unset wait data
    shared_data->wait.sem_state = SEM_NOT_INIT;
    shared_data->wait.process_count = 0;
    shared_data->wait.processes_passed = 0;

    return 0;
}

/**
 * Synchronises all processes in the list
 */
//extern void PT_SynchroniseAll(PTList *list)
//{
//    // checking if list is empty
//    if (list == NULL) {
//        fprintf(stderr, "ERROR - PT_SynchroniseAll, list is empty\n");
//        return;
//    }
//
//    // initialising semaphore
//    bool sem_init_process = false;
//
//    if (list->semaphores->synchronize_all.state == SEM_NOT_INIT) {
//        list->semaphores->synchronize_all.state = SEM_IN_INIT;
//        sem_init_process = true;
//    }
//
//    // initialising semaphore
//    if (list->semaphores->synchronize_all.state == SEM_IN_INIT) {
//
//        // count the active number of processes
//        unsigned int active_p = 0;
//
//        for (int i = 0; i < list->t_num; i++) {
//            for (int j = 0; j < list->t_arr[i].p_num; j++) {
//                if (list->t_arr[i].p_arr[j].state != DEAD) {
//                    active_p++;
//                }
//            }
//        }
//
//        // marking the active processes
//        if (sem_init_process == true) {
//            list->semaphores->synchronize_all.running_p = active_p;
//            list->semaphores->synchronize_all.state = SEM_INIT;
//            //printf("p_set_up = %d, active_p = %d, sleep_p = %d\n", getpid(), list->semaphores->synchronize_all.running_p, list->semaphores->synchronize_all.sleeping_p);
//            //fflush(stdout);
//        }
//    }
//
//
//
//    // stopping the process if it is not the last one active
//    if (((list->semaphores->synchronize_all.running_p - 1) <= list->semaphores->synchronize_all.sleeping_p) && (list->semaphores->synchronize_all.running_p > 0)) {
//        
//        // saving the number of sleeping processes
//        unsigned int sleeping_p = list->semaphores->synchronize_all.sleeping_p;
//        //printf("\np_wake = %d, active_p = %d, sleep_p = %d\n", getpid(), list->semaphores->synchronize_all.running_p, sleeping_p);
//        //fflush(stdout);
//
//        // resetting the semaphore
//        list->semaphores->synchronize_all.state == SEM_NOT_INIT;
//        list->semaphores->synchronize_all.running_p = 0;
//        list->semaphores->synchronize_all.sleeping_p = 0;
//
//        // waking up all the processes
//        for (int i = 0; i < sleeping_p; i++) {
//            sem_post(list->semaphores->synchronize_all.sem);
//        }
//
//    // waking up all the processes
//    } else {
//        //printf("p_sleep = %d, active_p = %d, sleep_p = %d\n", getpid(), list->semaphores->synchronize_all.running_p, list->semaphores->synchronize_all.sleeping_p);
//        //fflush(stdout);
//        list->semaphores->synchronize_all.sleeping_p++;
//        sem_wait(list->semaphores->synchronize_all.sem);
//    }
//}



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
    for (unsigned int i = 0; i < list->t_num; i++) {

        // printing tag
        printf("( %s ) ->", list->t_arr[i].key);

        // going through every process in tag
        for (unsigned int j = 0; j < list->t_arr[i].p_num; j++) {
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

/* Makes process sleep for random amount of miliseconds*/
int ran_msec_sleep(int min_msec, int max_msec)
{
    // initialising random seed
    static bool init = false;
    if (init == 0) {
        srand(getpid());
        init = true;
    }

    // generating random number
    long msec = (rand() % (max_msec - min_msec + 1)) + min_msec;

    // sleeping for random amount of miliseconds
    return msec_sleep(msec);
}