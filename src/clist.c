#include <clist.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/// @file clist.c

static PCLIST clist_reallocate_list(PCLIST);

/**
 * @brief Initialize a list and return the list context
 * @param free_func A pointer to the function responsible for freeing list items.
 * @return A pointer to list context on success or NULL on failure.
 *  
 * The 'free_func' pointer (parameter) is necessary to make sure there is
 * no leak at the time of using clist_free() function. If the list items
 * are strings, then you can just pass 'free()' from stdlib.h but if 
 * you are storing a customized structure as list items, then you need
 * to write a free function and pass it as a pointer to clist_init().
 *
 * Look at the example in 'test' directory.
 *
 */
PCLIST clist_init(void(*free_func)(void*)){
    PCLIST lst = (PCLIST) malloc(sizeof(CLIST));
    if (NULL == lst){
        fprintf(stderr, "Can not allocate memory for list!\n");
        return NULL;
    }
    void ** arr = (void *) malloc(sizeof(void*) * (LIST_MAX_SIZE + 1) );
    if (NULL == arr){
        fprintf(stderr, "Can not allocate memory for list!\n");
        free(lst);
        return NULL;
    }
    for (unsigned int i=0; i<LIST_MAX_SIZE; ++i)
        arr[i] = NULL;
    lst->_len = LIST_MAX_SIZE -1;
    lst->list = arr;
    lst->first_empty = 0;
    lst->free_func = free_func;
    lst->last_item = 0;
    return lst;
}

static PCLIST clist_reallocate_list(PCLIST plst){
    // having the previous pointer, reallocates more memory and return the pointer
    void ** tmp = (void*)realloc(plst->list, sizeof(void*) * (LIST_MAX_SIZE + 1 + plst->_len));
    if (NULL == tmp){
        fprintf(stderr, "Not able to reallocate memory!");
        return NULL;
    }
    plst->list = tmp;
    return plst;
}


/**
 * @brief Adds a new item to the provided list
 * @param plst A pointer to the list context returned by clist_init()
 * @param item A pointer to the item to be stored in the list.
 * @return 0 on success and 1 on failure.
 */
int clist_add_item(PCLIST plst, void * item){
    // adds new item to the list
    plst->list[plst->first_empty] = item;
    plst->last_item = plst->first_empty;
    plst->first_empty += 1;
    if (plst->first_empty % LIST_MAX_SIZE == 0){
        // we need to reallocate stuff
        if(clist_reallocate_list(plst) == NULL){
            fprintf(stderr, "Can not reallocate memory!\n");
            // can't allocate memory
            return 1;
        }
        plst->_len += LIST_MAX_SIZE;
    }
    return 0;
}


/**
 * @brief Free the list.
 * @param plst A pointer to the list context created by clist_init()
 * @return Nothing!
 */
void clist_free(PCLIST plst){
    if (NULL == plst)
        return;
    for (unsigned int i=0; i<= plst->last_item; ++i){
        plst->free_func(plst->list[i]);
        plst->list[i] = NULL;
    }
    free(plst->list);
    free(plst);
    plst = NULL;
}


//other methods must be implemented based on the type
