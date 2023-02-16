#include "process_table.h"

#define is_init_pid(PTList) (list->init_pid.pid == getpid())

int main(void) 
{
    // init process table
    PTList *list = PT_Init(5, 20);

    // create processes
    for (int i = 0; i < 10; i++) {
        PT_ProcessCreate(list, "red");
    }

    for (int i = 0; i < 5; i++) {
        PT_ProcessCreate(list, "blue");
    }

    for (int i = 0; i < 3; i++) {
        PT_ProcessCreate(list, "green");
    }
    
    // synchronize all processes
    msec_sleep(50);
    PT_SynchroniseAll(list);

    ran_msec_sleep(10000);

    printf("Hello World\n");

    
   
    return 0;
}