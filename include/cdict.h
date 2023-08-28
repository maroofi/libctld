///@file dictlib.h

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#ifndef CDICT_H
#define CDICT_H


#define ERROR_OK                        0      ///< No error. Everything is fine
#define CDICT_ERROR_CAN_NOT_ALLOCATE_MEMORY   1      ///< Can not allocate memory using malloc
#define ERROR_ITEM_NOT_FOUND            2      ///< Current key does not exist in the dictionary
#define ERROR_NULL_VALUE_FOR_KEY        3      ///< Key can not be NULL
#define ERROR_DICT_POINTER_IS_NULL      4      ///< invalid pointer to the dictionary structure

#define TABLE_SIZE 0x10001

typedef uint16_t  WORD;  ///< defines a fixed size (16 bit) cell.

typedef struct _NODE NODE, *PNODE;

/**
 * @details Structure of each node in the dictionary.
 */
struct _NODE{
    char * key;             ///< key for finding matches. Key is a null-terminated string
    void * value;           ///< Value to store. value is a pointer to void so it can be any type
    struct _NODE * next;    ///< A pointer points to the next node in case of collision
};


typedef struct _DICT cdict_ctx;

/**
 * @details This is the main structure to hold the dictionary.
 */
struct _DICT{
    PNODE table;                ///< Pointer to an array of #NODE structure
    WORD * hash_table;          ///< Pointer to the hash table for hashing
    char * errmsg;              ///< Description of the possible error
    int err;                    ///< Possible Error code
    void (*free_func)(void*);   ///< a pointer to the free function provided by user
    void *(*copy_func)(void*);  ///< a pointer to the copy function provided by user
};

typedef struct {
    char ** lst;
    unsigned int len;
} cdict_keylist;



/*start of function definitions*/
cdict_ctx * cdict_init(void(*free_func)(void*), void*(*copy_func)(void*));
void cdict_free(cdict_ctx*);
int cdict_has_key_nocase(cdict_ctx* ctx, char * key);
int cdict_set_nocase(cdict_ctx *ctx, char * key, void * value);
void * cdict_get(cdict_ctx*, char *);
void * cdict_get_nocase(cdict_ctx* ctx, char * key);
int cdict_set(cdict_ctx*, char *, void *);
void cdict_free_keylist(cdict_keylist*, int);
cdict_keylist * cdict_keys(cdict_ctx *ctx, int clone_keys);
int cdict_has_key(cdict_ctx*, char * key);
void cdict_remove(cdict_ctx *, char *);
/*end of function definitions*/
#endif

