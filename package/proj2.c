/**
 * @file proj2.c
 * @author Nikolas Nosál (xnosal01@stud.fit.vutbr.cz)
 * @brief Project created for course IOS (Operating Systems) at FIT BUT - Projekt 2 (synchronizace). 
 * @date 2023-04-24
 */

/* - - - - - - - - - - -*/
/*      DEFINITIONS     */
/* - - - - - - - - - - -*/

/* libraries */
#include "process_table.h"

/* functions */
int parse_arguments(int argc, char *argv[], int arg_array[], int arg_num);
int ran_num(int min_num, int max_num);

/* constants */
#define PROGRAM_NAME "proj2.c"
#define ARG_NUM 5
#define P_TYPE_NUM 2



/* - - - - - - - - - - -*/
/*         MAIN         */
/* - - - - - - - - - - -*/

int main(int argc, char *argv[]) 
{
    // [0] - program parses arguments and opens a log file
    // parse arguments and them to array
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
    int arg_f  = arg_arr[4];    // max amount of post office is going to be open in miliseconds

    // open log file proj2.out
    FILE *log_file = fopen("proj2.out", "w");

    // check if the log file was opened
    if (log_file == NULL) {
        fprintf(stderr, "[%s] - Error while opening log file\n", PROGRAM_NAME);
        return 1;
    }


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
    err_ret = SM_OfficeInit(list->shared_data);

    // check if the shared memory data was initialized
    if (err_ret != 0) {
        fprintf(stderr, "[%s] - Error while initializing semaphores\n", PROGRAM_NAME);
        PT_Destroy(&list);
        return 1;
    }


    // [2] - main process creates nz number of customer processes and nu number of officer processes
    // process data variable declarations
    char buffer[BUFFER_SIZE] = {0};
    int tag_num = 0, pro_num = 0;
    
    // create officer/uradnik processes  U
    for (int i = 0; i < arg_nz; i++) {
        if (is_init_pid(list)) {
            tag_num = 0, pro_num = i;       // save the process number
            PT_ProcessCreate(list, "Z");    // create the process
        } else {
            break;
        }
    }

    // create customer/zakaznik processes Z 
    for (int i = 0; i < arg_nu; i++) {
        if (is_init_pid(list)) {
            tag_num = 1, pro_num = i;       // save the process number
            PT_ProcessCreate(list, "U");    // create the process
        } else {
            break;
        }
    }  


    // [3] - then main process sleeps for random ammount of time between f/2 and f miliseconds
    if (is_init_pid(list)) {
        tag_num = 2, pro_num = 0;    // differentiate main process 

        // sleep for random ammount of time between f/2 and f miliseconds
        if (ran_msec_sleep(arg_f/2, arg_f) != 0) {
            fprintf(stderr, "[%s] - Sleep function has been canceled\n", PROGRAM_NAME);
            return 1;
        }

        // the main process prints "A: closing\n"
        SM_OfficeClose(list->shared_data);
        SM_CounterPrint(list->shared_data, log_file, "closing");
    }


    // [4] - Customer processes are going to the post office
    if (tag_num == 0) {

        // print process started
        sprintf(buffer, "Z %d: started", pro_num);
        SM_CounterPrint(list->shared_data, log_file, buffer);
        
        // wait random ammount of time in interval <0, tz>
        ran_msec_sleep(0, arg_tz);

        // customer is going to the post office
        if (list->shared_data->office.is_open == 1) {
        
            // choosing service <1,3>
            int service = ran_num(1,3);

            // print is chosing service n
            sprintf(buffer, "Z %d: entering office for a service %d", pro_num, service);
            SM_CounterPrint(list->shared_data, log_file, buffer);

            // goes to front with service type <n>
            SM_OfficeService(list->shared_data, log_file, pro_num, service);
        } 

        // customer is going home
        sprintf(buffer, "Z %d: going home", pro_num);
        SM_CounterPrint(list->shared_data, log_file, buffer);
    }


    // [5] - Officer processes are going to the post office
    if (tag_num == 1) {

        // print process started
        sprintf(buffer, "U %d: started", pro_num);
        SM_CounterPrint(list->shared_data, log_file, buffer);

        // cycle until the post office is closed
        while (list->shared_data->office.is_open == 1 || list->shared_data->office.sem_1_count > 0 
                || list->shared_data->office.sem_2_count > 0 || list->shared_data->office.sem_3_count > 0) {

            // go to the random front and serve customers
            SM_OfficeServe(list->shared_data, log_file, pro_num, arg_tu);
        }

        // officer is going home
        sprintf(buffer, "U %d: going home", pro_num);
        SM_CounterPrint(list->shared_data, log_file, buffer);
    }


    // [6] main process waits for all processes and destroys allocated data
    if (is_init_pid(list)) {

        // wait for all processes to finish
        for (int i = 0; i < (arg_nz + arg_nu); i++) {
            wait(NULL);
        }

        // destroy existing data structures
        SM_CounterDestroy(list->shared_data);
        SM_OfficeDestroy(list->shared_data);
        PT_Destroy(&list);
    }
    return 0;
}



/* - - - - - - - - - - -*/
/*      FUNCTIONS       */
/* - - - - - - - - - - -*/

/**
 * Function parses arguments into an array of integer values. The function also checks if the arguments
 * are in the correct format specified in [(2022/2023 - IOS – projekt 2 (synchronizace)] assigment.
 * 
 * @param argc Integer value of the number of arguments
 * @param argv Array of strings which contains the arguments
 * @param arg_arr (return) Array of integers which contains the parsed arguments
 * @param arg_num (return) Number of parsed arguments
 * @return int Function returns(0) if the arguments are in the correct format, otherwise returns(1)
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

            // NU - arg has to be > 0
            case 1: 
                if (arg_arr[i] <= 0) {
                    return 1;
                }
                break;
        
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

/**
 * Function returns random number in range <min_num, max_num>.
 * 
 * @param min_num Minimum number in range
 * @param max_num Maximum number in range
 * @return int 
 */
int ran_num(int min_num, int max_num)
{
    // initialising random seed defined by process id
    static bool init = false;
    if (init == 0) {
        srand(getpid());
        init = true;
    }

    // generating random number
    return ((rand() % (max_num - min_num + 1)) + min_num);
}