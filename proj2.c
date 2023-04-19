
#include <stdlib.h>

#include "process_table.h"

int parse_arguments(int argc, char *argv[], int arg_array[], int arg_num);

#define PROGRAM_NAME "proj2.c"
#define ARG_NUM 5
#define P_TYPE_NUM 2


/**
 * @brief 
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[]) 
{
    // [0] - program parses arguments
    // parse arguments
    int arg_arr[ARG_NUM];
    if (parse_arguments(argc, argv, arg_arr, ARG_NUM) != 0) {
        fprintf(stderr, "[%s] - Wrong arguments\n", PROGRAM_NAME);
        return 1;
    }

    // assign arguments to variables
    int arg_nz = arg_arr[0];    // number of customers
    int arg_nu = arg_arr[1];    // number of officers
    int arg_tz = arg_arr[2];    // max ammount of time customer is going to the post office in miliseconds
    int arg_tu = arg_arr[3];    // max ammount of time officer can take a break in miliseconds
    int arg_f = arg_arr[4];     // max amount of post office is going to be open in miliseconds

    // debug -> print arguments
    //for (int i = 0; i < ARG_NUM; i++) {
    //    printf("arg[%d] = %d\n", i, arg_arr[i]);
    //}



    // [1] - program creates shared memory for process table and initialize semaphores    
    // get max number of processes and create process table with max number of processes
    int max_p_num = (arg_nz > arg_nu) ? arg_nz : arg_nu;  
    PTList *list = PT_Init(P_TYPE_NUM, max_p_num);             
    
    // check if the process table was created
    if (list == NULL) {
        fprintf(stderr, "[%s] - Error while creating process table\n", PROGRAM_NAME);
        return 1;
    }

    // initialze shared memory data
    int err_ret;
    err_ret = SM_CounterInit(list->shared_data);
    err_ret = SM_WaitInit(list->shared_data, list->init_pid.pid, (arg_nz + arg_nu));

    // check if the shared memory data was initialized
    if (err_ret != 0) {
        fprintf(stderr, "[%s] - Error while initializing semaphores\n", PROGRAM_NAME);
        return 1;
    }



    // [2] - main process creates nz number of customer processes and nu number of officer processes
    // create customer processes
    for (int i = 0; i < arg_nz; i++) {
        PT_ProcessCreate(list, "U");
    }

    // create officer processes
    for (int i = 0; i < arg_nu; i++) {
        PT_ProcessCreate(list, "Z");
    }    



    // [3] - then main process sleeps for random ammount of time between f/2 and f miliseconds
    if (is_init_pid(list)) {

        // sleep for random ammount of time between f/2 and f miliseconds
        if (ran_msec_sleep(arg_f/2, arg_f) != 0) {
            fprintf(stderr, "[%s] - Sleep function has been canceled\n", PROGRAM_NAME);
            return 1;
        }

        // the main process prints "A: closing\n"
        SM_CounterPrint(list->shared_data, "closing");
    }  

    

    // debug
    ran_msec_sleep(0, 2000);




    //[4] - main process waits for all processes to finish and then destroys all semaphores and shared memory
    // main process, waits for all processes to finish
    SM_WaitForAll(list->shared_data);
        
    // destroy existing data structures
    if (is_init_pid(list)) {
        
        // debug -> print process table
        PT_PrintList(list);

        // destroy existing data structures
        SM_CounterDestroy(list->shared_data);
        SM_WaitDestroy(list->shared_data);
    
    } else {

        // debug -> print process id
        char buffer[100] = {0};
        sprintf(buffer, "Ending process = %d", getpid());
        SM_CounterPrint(list->shared_data, buffer);
    }

    return 0;
}


/**
 * @brief 
 * 
 * @param argc 
 * @param argv 
 * @param arg_arr 
 * @param arg_num 
 * @return int 
 */
int parse_arguments(int argc, char *argv[], int arg_arr[], int arg_num)
{   
    // wrong number of arguments
    if (argc != (arg_num + 1)) {
        return 1;
    }

    // parse and check the arguments
    for (int i = 1; i < argc; i++) {
        
        // check if the argument is a number
        char *endptr;
        arg_arr[i-1] = strtol(argv[i], &endptr, 10);
        
        // if the argument is not a number return 1
        if (*endptr != '\0') {
            return 1;
        }
    }

    // check arg specific conditions
    for (int i = 0; i < arg_num; i++) {
        switch (i) {
        
            // TZ - arg is in range 0-10000
            case 2:
                if ((arg_arr[i] > 10000) || (arg_arr[i] < 0)) {
                    return 1;
                }
                break;

            // TU - arg is in range 0-100
            case 3:
                if ((arg_arr[i] > 100) || (arg_arr[i] < 0)) {
                    return 1;
                }
                break;

            // F - arg is in range 0-10000
            case 4:
                if ((arg_arr[i] > 10000) || (arg_arr[i] < 0)) {
                    return 1;
                }
                break;

            default:
                break;
        }
    }

    // return the array
    return 0;
}