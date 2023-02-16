/**
 *  AUTOR : Nikolas Nosál 
 * 
 *  DESCRIPTION : Show-case of custom made functions 
 * 
**/

// standart libraries
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// linux libs
#include <sys/types.h>
#include <unistd.h>

// custom libs
#include "process_table.h"

#define PID_SIZE 5


int main(void) 
{
    PTList* list = PT_Init();

    PT_ProcessCreate(list, "bruh");
    printf("was here\n");
    PT_ProcessCreate(list, "bruh");
    printf("pid = %d\n", getpid());

    msec_sleep(1000);
    PT_PrintList(list);

    msec_sleep(1000);
    printf("pid = %d\n", getpid());
   
    msec_sleep(10000);
    return 0;
}