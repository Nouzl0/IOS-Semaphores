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
extern PTList* PT_Init(void) 
{      
    // creating list struct
    PTList* list = mmap(NULL, sizeof(PTList), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_Init, mmap failed (PTList)\n");  
        return NULL;
    }

    // tag list
    list->firstTag = NULL;

    // writing the size of the array
    list->init_pid = getpid();
    list->tag_num = 0;
    list->process_num = 0;

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
 * 
 * @param list Pointer to PTlist
 * @param string Key of the element which is searched
 * @return Pointer to PTProcess or NULL if unsuccessful
 */
//extern PTProcessPtr ST_ProcessSearch(PTList *list, char *string) 
//{
//    // if list is empty
//    if (list == NULL) {
//        fprintf(stderr, "ERROR - PT_ProcessSearch, list is empty\n");
//        return NULL;
//    }
//
//    int hash_value = ST_Hash(string, list->list_size);
//    PTProcessPtr element_p = list->array[hash_value];
//
//    while (element_p != NULL) {
//        if ((strcmp(element_p->key, string)) == 0) {
//            return element_p;
//        }
//
//        element_p = element_p->nextElement;
//    }
//    
//    fprintf(stderr, "ERROR - PT_ProcessSearch, %s doesnt't exist\n", string);
//    return element_p;
//}

/**
 * Creates a process in process table with no data
 * 
 * @param list Pointer to PTlist
 * @param tag key and key of the new element 
 */
extern void PT_ProcessCreate(PTList *list, char *tag)
{   
    if (getpid() != list->init_pid) {
        return;
    }

    // checking if list is not empty
    if (list == NULL) {
        fprintf(stderr, "ERROR - PT_ProcessCreate, list is empty\n");
        return;
    }

    // checking if new tag has to be created and where it is when it exists
    bool tag_exists = false;
    PTListTagPtr tag_ptr = list->firstTag;

    while (tag_ptr != NULL) {
        if (strcmp(tag_ptr->key, tag) == 0) {
            tag_exists = true;
            break;
        }
         
        if (tag_ptr->nextTag != NULL) {
            tag_ptr = tag_ptr->nextTag;
        } else {
            break;
        }
    } 

    // creating new process data
    PTProcessPtr new_process = mmap(NULL, sizeof(struct PTProcess), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (new_process == NULL) {
        fprintf(stderr, "ERROR - PT_ProcessCreate, mmap failed (PTProcess)\n");
        return;
    }

    // linking or creating new tag
    if (tag_exists == false) {

        PTListTagPtr new_tag = mmap(NULL, sizeof(struct PTListTag), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (new_tag == NULL) {
            fprintf(stderr, "ERROR - PT_ProcessCreate, mmap failed (PTListTag)\n");
            return;
        }

        // if list is empty set the new tag as first tag
        if (list->firstTag == NULL) {
            list->firstTag = new_tag;
        } else {
            tag_ptr->nextTag = new_tag;
        }


        // setting up the new tag data
        new_tag->firstProcess = new_process;
        new_tag->firstProcess->nextProcess = NULL;
        new_tag->nextTag = NULL;
        strcpy(new_tag->key, tag);
        list->tag_num++;

    // linking the new process to the existing tag
    } else {
        new_process->nextProcess = tag_ptr->firstProcess;
        tag_ptr->firstProcess = new_process;
    }

    pid_t id = fork();

    // setting up the new process data
    if (id == 0) {
        new_process->pid = getpid();
        new_process->ppid = getppid();
        new_process->state = 2;
        list->process_num++;
    }

}


/**
 * Deletes an element in symbol 
 * - uses function [ST_ElementExists()]
 * 
 * @param list Pointer to STlist
 * @param string key and key of the element 
 */
//extern void PT_ProcessDelete(STList *list, char *string )
//{
//    // linking pointers of element
//    STElementPtr element_p, behind_p;
//
//    // checks if element exists
//    if (!ST_ElementExists(list, string) ) {
//        fprintf(stderr,"ERROR - ST_Delete, %s doesn't exists or list is empty\n", string);
//        return;
//    }
//
//    // hash of element
//    int hash_value = ST_Hash(string, list->list_size);
//
//    // searched element is the first one
//    if ( (strcmp(list->array[hash_value]->key, string)) == 0) {
//        
//        element_p = list->array[hash_value]->nextElement;
//        
//        // deleting element
//        free(list->array[hash_value]->data);
//        free(list->array[hash_value]);
//
//        // relinking element
//        list->array[hash_value] = element_p;
//    
//    
//    // searched element is in the linked list
//    } else {
//        element_p = list->array[hash_value];
//
//        while (element_p != NULL) {
//            if ( (strcmp(element_p->key, string)) == 0) {
//               break;
//            } else {
//                behind_p = element_p;
//                element_p = element_p->nextElement;
//            }
//        }  
//
//        // relinking elements
//        behind_p->nextElement = behind_p->nextElement->nextElement;
//
//        // deleting the targeted element
//        free(element_p->data);
//        free(element_p);
//    }
//}

/**
 * Deletes every element in the symbol table
 * 
 * @param list Pointer to STlist
 */
//extern void PT_DeleteAll(STList *list) 
//{   
//
//    if (list == NULL) {
//        fprintf(stderr, "ERROR - ST_DeleteAll, list is empty\n");
//        return;
//    }
//
//    // tmp STElement pointer
//    STElementPtr element_p;
//
//    // going through every array pointer of STlist
//    for (int i = 0; i < (list->list_size); i++) {
//        
//        // deleting the elements in linked list
//    	while (list->array[i] != NULL) {
//    		element_p = list->array[i]->nextElement;
//    		
//            // deleting data
//            free(list->array[i]->data);
//            free(list->array[i]);
//    		
//            // moving to next element
//            list->array[i] = element_p;
//    	}
//
//    	// list init
//    	list->array[i] = NULL;
//    }
//}


/* - - - - - - - - - - - - - - - */
/*   ST_ELEMENT_DATA FUNCTIONS   */
/* - - - - - - - - - - - - - - - */






/* - - - - - - - - - - */
/*   DEBUG FUNCTIONS   */
/* - - - - - - - - - - */

/**
 * Prints the list (debug)
 */
extern void PT_PrintList(PTList *list)
{   
    // check if list is empty
    if (list == NULL) {
        return;
    }

    // tmp pointers
    PTListTagPtr tag_ptr = list->firstTag;
    PTProcessPtr process_ptr = NULL;

    printf("[ pid: %d, pid_num: %ld ]\n", list->init_pid, list->process_num);
    printf("getpid(): %d\n", getpid());
    // printing tags in linked list
    while(tag_ptr != NULL) {
        printf(" %s -> ", tag_ptr->key);

        process_ptr = tag_ptr->firstProcess;

        // printing processes in linked list
    	while (process_ptr != NULL) {

            // printing the element
            printf("[ %d, %d ", process_ptr->pid, process_ptr->state);
            printf("] -> ");

            // moving to next process node
            process_ptr = process_ptr->nextProcess;
    	}

        // moving to next tag node
        printf("NULL\n");
        tag_ptr = tag_ptr->nextTag;
    }

    printf("- - - - \n\n");
}

/**
 * Prints test number, used for boxing tests into bundles (debig)
 */
extern void PT_PrintHeader(char *string) 
{   
    // testing 
    if (string == NULL) {
        fprintf(stderr, "ERROR - ST_PrintHeader - when accesing string found only NULL");
        return;
    }

    // test counter
    static int test_num = 1;

    // printing the text
    printf("\n\n| - - - - - - - - - - - - - - - - -\n");
    printf("| - TEST %d - %s", test_num, string);
    printf("\n| - - - - - - - - - - - - - - - - -\n\n");

    test_num++;
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

static int get_shmem_block(char *filename, int size) 
{
    // requesting key, key is linked to a filename for access
    key_t key = ftok(filename, 0);
    if (key == IPC_RESULT_ERROR) {
        fprintf(stderr, "ERROR - get_shmem_block, ftok failed\n");
        return IPC_RESULT_ERROR;
    }

    // get shared memory block
    return shmget(key, size, 0644 | IPC_CREAT);
}

void* attach_shmem_block(char *filename, int size) 
{
    // get shared memory block
    int shared_block_id = get_shmem_block(filename, size);
    char *result;

    // check errors
    if (shared_block_id == IPC_RESULT_ERROR) {
        fprintf(stderr, "ERROR - attach_shmem_block, get_shmem_block failed\n");
        return NULL;
    }

    // map the shared block to the process address space, returns pointer to the shared block
    result = shmat(shared_block_id, NULL, 0);
    if (result == (char *) IPC_RESULT_ERROR) {
        fprintf(stderr, "ERROR - attach_shmem_block, shmat failed\n");
        return NULL;
    }

    return result;
}

bool detach_shmem_block(char *block) 
{
    // check errors
    if (block == NULL) {
        fprintf(stderr, "ERROR - detach_shmem_block, block is NULL\n");
        return false;
    }

    // detach the shared block from the process address space
    if (shmdt(block) == IPC_RESULT_ERROR) {
        fprintf(stderr, "ERROR - detach_shmem_block, shmdt failed\n");
        return false;
    }

    return true;
}

bool destroy_memory_block(char *filename) 
{
    int shared_block_id = get_shmem_block(filename, 0);

    if (shared_block_id == IPC_RESULT_ERROR) {
        fprintf(stderr, "ERROR - destroy_memory_block, get_shmem_block failed\n");
        return false;
    }

    if (shmctl(shared_block_id, IPC_RMID, NULL) == IPC_RESULT_ERROR) {
        fprintf(stderr, "ERROR - destroy_memory_block, shmctl failed\n");
        return false;
    }
}