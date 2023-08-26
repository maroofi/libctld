/** @file */
#ifndef _CLIST_H
#define _CLIST_H

#define ERROR_OK 0
#define LIST_ITEM_NOT_FOUND 1


#define LIST_MAX_SIZE 0xFF

/**
* @details Type definition of the struct _CLIST
* 
* CLIST: structure of type _CLIST
* 
* PCLIST: Pointer to the structure of type _CLIST
* 
*/
typedef struct _CLIST CLIST, *PCLIST;

/**
 * @details This structure is created by the clist_init() function as a list context.
 * 
 * The structure must be passed to all the functions to operate on this list.
 */
struct _CLIST{
    unsigned long int _len;     ///< for internal use only to get the curren length
    void ** list;               ///< The actual array of items for the list
    unsigned long int first_empty;  ///< first empty cell of the list
    unsigned long int last_item;    ///< index of the last item in the list for iteration
    void (*free_func)(void*);       ///< A pointer to the free function passed by user
};


PCLIST clist_init(void(*free_func)(void*));
int clist_add_item(PCLIST, void *);
void clist_free(PCLIST);

#endif
