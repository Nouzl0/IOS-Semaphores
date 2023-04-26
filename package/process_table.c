/** @file process_table.c
 *  @author Nikolas Nosál (xnosal01@stud.fit.vutbr.cz)
 *  @date 2023-04-24
 */

#include "process_table.h"



/* - - - - - - - - - - - - */
/*   PT_PROCESS FUNCTIONS  */
/* - - - - - - - - - - - - */
// Functions which work with process data nodes (PTList)

/**
 * Function creates (PTList), which is an list with processes and it's data. PTList is a struct with array of tags where, 
 * each tag contains an array of process data nodes which can contain data about the processes. PTList also contains shared data
 * of which used by the other functions. Shared data is a data module specifically created for this project.
 * 
 * @param list Pointer to PTlist
 * @param t_size Number of tag nodes which will be created
 * @param p_size Number of process data nodes which will be created
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
 * Deallocates the PTList and all it's data.
 * 
 * @param list Pointer to PTlist, which will be deallocated.
 */
extern int PT_Destroy(PTList **list) 
{
    // checking if list is empty
    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_Destroy, list is empty\n");
        return 1;
    }

    // checking if process is the list proceess
    if (getpid() != (*list)->init_pid.pid) {
        return 0;
    }

    // deallocating all the shared memory
    int err_val = 0;
    err_val += munmap((*list)->t_arr->p_arr, (*list)->p_size * sizeof(struct PTListData));
    err_val += munmap((*list)->t_arr, (*list)->t_size * sizeof(struct PTListTag));
    err_val += munmap((*list)->shared_data, sizeof(struct PTListData));
    err_val += munmap(*list, sizeof(PTList));

    // setting list to NULL
    *list = NULL;
    
    // error handling
    if (err_val != 0) {
        fprintf(stderr, "ERROR - PT_Destroy, munmap failed\n");
        return 1;
    }

    return 0;
}

/**
 * Function checks if the process which passes the function is the process which is saved in the PTList under the given tag.
 * 
 * @param list Pointer to PTlist
 * @param tag String with the tag
 * @return Returns true(1) if the process is in the list under the given tag or false(0) if not.
 */
extern int PT_IsTag(PTList *list, char *tag) {

    // checking if list is empty
    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_IsTag, list is empty\n");
        return 0;
    }
    
    // getting process ID
    pid_t searched_pid = getpid();

    // searching for tag in the list
    for (unsigned int i = 0; i < list->t_num; i++) {
        if (strcmp(list->t_arr[i].key, tag) == 0) {
            
            // search for process in the tag
            for (unsigned int j = 0; j < list->t_arr[i].p_num; j++) {
                if (list->t_arr[i].p_arr[j].pid == searched_pid) {
                    return 1;
                }
            }
        }        
    }
    return 0;
}

/**
 * Function which searches for the process in the PTList and returns data about the process, with confirmation if 
 * the process is in the list.
 * 
 * @param list Pointer to PTList
 * @param pid Process ID of the searched process
 * @param tag (return), pointer to process tag
 * @param tag_num (return), pointer to number of tag in the tag array
 * @param pro_num (return), pointer to number of process in a process array
 * @return int Function returns(1) if the process is in the list or returns(0) if not.
 */
extern int PT_ProcessSearch(PTList *list, pid_t pid, char *tag, int *tag_num, int *pro_num)
{
    // checking if table is empty
    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_ProcessSearch, list is empty\n");
        return 0;
    }

    // check if the given pointers are not NULL
    if (tag == NULL || tag_num == NULL || pro_num == NULL) {
        fprintf(stderr, "ERROR - PT_ProcessSearch, one of given pointers is NULL\n");
        return 0;
    }

    // searching for tag in the list
    for (unsigned int i = 0; i < list->t_num; i++) {          
        // search for process in the tag
        for (unsigned int j = 0; j < list->t_arr[i].p_num; j++) {
            if (list->t_arr[i].p_arr[j].pid == pid) {
                
                // returning data
                tag = list->t_arr[i].key;
                *tag_num = i;
                *pro_num = j;
                return 1;
            }
        }
    }        
    
    // process not found
    fprintf(stderr, "ERROR - PT_ProcessSearch, process not found\n");
    return 0;
}

/**
 * Function which creates a new process and adds data about the process to the PTList.
 * 
 * @param list Pointer to PTList
 * @param tag Tag of the process which will be assigned to the new created process
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
 * Process will print all the data in the PTlist. Used for debugging.
 * 
 * @param list Pointer to PTList.
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



/* - - - - - - - - - - - */
/*       SM_COUNTER      */
/* - - - - - - - - - - - */
// Shared memory - functions which use counter data
// These functions are used by main() and office functions

/**
 * Initializes counter data and semaphores. This function must be called before using any other function.
 * Also, this function must be called before creating new processes, memory is allocated through mmap().
 * 
 * @param shared_data Pointer to shared_data.
 * @return int returns(0) if the function intiliazes corectly, returns(-1) if the initiliazation fails.
 */
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
    if (sem_init(&(shared_data->cnt.sem_1), 1, 1) == -1) {
        fprintf(stderr, "ERROR - SM_CounterInit, sem_init failed\n");
        return -1;
    }

    return 0;
}

/**
 * Function which prints message with counter data and increments the counter.
 * 
 * @param shared_data Pointer to shared_data.
 * @param file Pointer to file where the data will be printed
 * @param message Message which will be printed with counter data in the format: "counter: message"
 * @return int returns(0) if the function intiliazes corectly, returns(-1) if the initiliazation fails.
 */
int SM_CounterPrint(PTListDataPtr shared_data, FILE *file, char *message)
{
    // [0] - SEM-WAIT
    // lost processes are availabled by this data-struct
    //if (shared_data->cnt.sem_wait_count == 2) {
    //    sem_post(&(shared_data->cnt.sem_1));
    //    ran_msec_sleep(50, 150);
    //}

    // increasing number of sleeping processes
    shared_data->cnt.sem_wait_count++;

    if (sem_wait(&(shared_data->cnt.sem_1)) == -1) {
        fprintf(stderr, "ERROR\n");
    }

    // [1] - PRINT
    // print message and increment counter
    fprintf(file, "%d: %s\n", shared_data->cnt.data++, message);
    fflush(file);

    // [2] - SEMPOST
    // decresing the number of processes
    shared_data->cnt.sem_wait_count--;

    if (sem_post(&(shared_data->cnt.sem_1)) == -1) {
        fprintf(stderr, "ERROR - SM_CounterPrint, sem_post failed\n");
        return -1;
    }

    return 0;
}

/**
 * Function destroys counter data and semaphores.
 * 
 * @param shared_data Pointer to shared_data.
 * @return int return(0) if the semaphore is destroyed correctly, otherwise returns(-1)
 */
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



/* - - - - - - - - - - - - */
/*        SM_OFFICE        */
/* - - - - - - - - - - - - */
// Functions which implements the functionality of project IOS-Synchronizace 2022/2023

/**
 * Intializes office data and semaphores. This function must be called before using any other function.
 * Also, this function must be called before creating new processes.
 * 
 * @param shared_data Pointer to shared_data.
 * @return int returns(0) if the function intiliazes corectly, returns(-1) if the initiliazation fails.
 */
int SM_OfficeInit(PTListDataPtr shared_data)
{
    // initialize all semaphores
    int err_check = 0;
    err_check += sem_init(&(shared_data->office.sem_1), 1, 0);
    err_check += sem_init(&(shared_data->office.sem_2), 1, 0);
    err_check += sem_init(&(shared_data->office.sem_3), 1, 0);

    if (err_check != 0) {
        fprintf(stderr, "ERROR - SM_OfficeInit, sem_init failed\n");
        return -1;
    }

    // initialising data
    shared_data->office.is_open = 1;
    shared_data->office.sem_1_count = 0;
    shared_data->office.sem_2_count = 0;
    shared_data->office.sem_3_count = 0;

    return 0;
}

/**
 * Function closes the office, sets the office.is_open to closed state (0).
 * 
 * @param shared_data Pointer to shared_data.
 */
void SM_OfficeClose(PTListDataPtr shared_data)
{
    // closing office
    shared_data->office.is_open = 0;
}

/**
 * Function destroys office data and semaphores.
 * 
 * @param shared_data Pointer to shared_data.
 * @return int return(0) if the semaphore is destroyed correctly, otherwise returns(-1) 
*/
int SM_OfficeDestroy(PTListDataPtr shared_data)
{

    // unset data
    shared_data->office.is_open = 0;
    shared_data->office.sem_1_count = 0;
    shared_data->office.sem_2_count = 0;
    shared_data->office.sem_3_count = 0;

    // destroy semaphores
    int err_check = 0;
    err_check += sem_destroy(&(shared_data->office.sem_1));
    err_check += sem_destroy(&(shared_data->office.sem_2));
    err_check += sem_destroy(&(shared_data->office.sem_3));

    return 0;
}

/**
 * Function used by SM_OfficeServe function. Process which calls this function takes a break, is put to usleep.
 * 
 * @param shared_data Pointer to shared_data.
 * @param log_file Pointer to file where the data will be printed
 * @param process_id Process identifier, but it's not pid_t, it's an another indentification number.
 * @param max_break_time Maximum random time which the process will be put to sleep.
 * @return int return(0) if the break occured correctyl, otherwise returns(-1) 
 */
int SM_OfficeBreak(PTListDataPtr shared_data, FILE *log_file, int process_id, unsigned int max_break_time)
{
    char buffer[100] = {0};
    int err_value = 0;

    // take a break
    sprintf(buffer, "U %d: taking break", process_id);
    err_value += SM_CounterPrint(shared_data, log_file, buffer);

    // sleep for random time
    err_value += ran_msec_sleep(0, max_break_time);

    // break is over
    sprintf(buffer, "U %d: break finished", process_id);
    err_value += SM_CounterPrint(shared_data, log_file, buffer);

    // check if there was an error
    if (err_value != 0) {
        fprintf(stderr, "ERROR - SM_OfficeBreak - Error occured\n");
        return -1;
    }

    return 0;
}

/**
 * Process which calls this function serves a service to a customer process. (In form of messages).
 * This function should be called by officer type process. 
 * 
 * @param shared_data Pointer to shared_data.
 * @param log_file Pointer to file where the data will be printed
 * @param process_id Process identifier, but it's not pid_t, it's an another indentification number.
 * @param max_break_time maximum ammount of time which the process will be put to sleep if it takes a break.
 * @return int return(0) if the service was served correctly, otherwise returns(-1) 
 */
int SM_OfficeServe(PTListDataPtr shared_data, FILE *log_file, int process_id, unsigned int max_break_time)
{
    // check what kind of service is requested
    int type = 0, num = 0;

    // there should be mutex
    // choosing the service which is going to be served
    if (shared_data->office.sem_1_count > shared_data->office.sem_2_count) {
        num = shared_data->office.sem_1_count;
        type = 1;
    }  else {
        num = shared_data->office.sem_2_count;
        type = 2;
    }   

    if (shared_data->office.sem_3_count > num) {
        num = shared_data->office.sem_3_count;
        type = 3;
    }

    // writing which service is going to be served
    if (num > 0) {
        switch (type) {
            case 1: shared_data->office.sem_1_count--;
                break;
            case 2: shared_data->office.sem_2_count--;
                break;
            case 3: shared_data->office.sem_3_count--;
                break;
            default: break;
        }
    }
    // mutex should end here

    // if there is no one waiting, officer takes a break
    if (num <= 0) {
        SM_OfficeBreak(shared_data, log_file, process_id, max_break_time);

    // officer serves the service
    } else {

        // print which service is going to be served
        char buffer[100] = {0};
        sprintf(buffer, "U %d: serving a service of type %d", process_id, type);
        SM_CounterPrint(shared_data, log_file, buffer);

        // calculate how long it takes to serve the service, interval <0, 10> milisec
        unsigned int time = rand() % 11;    // this time will be sent to the customer process

        // synchronise with targeted customer service front
        switch (type) {
            case 1: 
                shared_data->office.timeout_1 = time;
                sem_post(&(shared_data->office.sem_1));
                break;
            case 2: 
                shared_data->office.timeout_2 = time;
                sem_post(&(shared_data->office.sem_2));
                break;
            case 3: 
                shared_data->office.timeout_3 = time;
                sem_post(&(shared_data->office.sem_3));
                break;
            default: break;
        }
    
        // work on the service
        msec_sleep(time);
        
        // print that service is done
        sprintf(buffer, "U %d: service finished", process_id);
        SM_CounterPrint(shared_data, log_file, buffer);
    }

    return 0;
}

/**
 * Process which calls this function get's serverd a service which is requested by officer process. (In form of messages).
 * This function should be called by customer type process. 
 * 
 * @param shared_data Pointer to shared_data.
 * @param log_file Pointer to file where the data will be printed
 * @param process_id Process identifier, but it's not pid_t, it's an another indentification number.
 * @param type_of_service Type of service which is requested by the process.
 * @return int return(0) if the process was served correctly, otherwise returns(-1) 
 */
int SM_OfficeService(PTListDataPtr shared_data, FILE *log_file, int process_id, int type_of_service)
{
    //go to the front of the queue
    switch (type_of_service) {
        case 1:
            shared_data->office.sem_1_count++;
            sem_wait(&(shared_data->office.sem_1));
            break;
        case 2:
            shared_data->office.sem_2_count++;
            sem_wait(&(shared_data->office.sem_2));
            break;
        case 3:
            shared_data->office.sem_3_count++;
            sem_wait(&(shared_data->office.sem_3));
            break;
        default: 
            fprintf(stderr, "ERROR - SM_OfficeService, wrong type of service\n");
            return -1;
    }

    // print that customer is being served
    char buffer[100] = {0};
    sprintf(buffer, "Z %d: called by office worker", process_id);
    SM_CounterPrint(shared_data, log_file, buffer);

    // wait for the service to be done
    switch (type_of_service) {
        case 1:
            msec_sleep(shared_data->office.timeout_1);
            break;
        case 2: 
            msec_sleep(shared_data->office.timeout_2);
            break;
        case 3: 
            msec_sleep(shared_data->office.timeout_3);
            break;
        default: break;
    }

    return 0;
}



/* - - - - - - - - - - */
/*   SLEEP FUNCTIONS   */
/* - - - - - - - - - - */
// functions which put process to sleep state

/**
 * Makes process sleep for n amount of miliseconds, implemented through usleep()
 * 
 * @param msec an amount of miliseconds process will sleep
 * @return int return(0) if the sleep functioned correctly, otherwise returns(-1)  
 */
int msec_sleep(long msec)
{
    int res = -1;
    res = usleep(1000 * msec);
    return res;
}

/**
 *  Makes process sleep for random amount of miliseconds in range <min_msec, max_msec>
 * 
 * @param min_msec a minimum amount of miliseconds process will sleep
 * @param max_msec a maximum amount of miliseconds process will sleep
 * @return int return(0) if the sleep functioned correctly, otherwise returns(-1)  
 */
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