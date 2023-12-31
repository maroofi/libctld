///@file cdict.c

#include <cdict.h>

/*declare static functions*/
static void free_hash_table(WORD * hash_table);
static int cto_lower(int c);
static WORD * init_hash_table(void);
static void shuffle(WORD * table, unsigned int);
static unsigned int hashme(cdict_ctx*, char *);
static int cstr_ccmp(const char * str1, const char * str2);
/*****************************************/


static int cto_lower(int c){
    return c >= 'A' && c <= 'Z'? c + 'a' - 'A':c;
}

static int cstr_ccmp(const char * str1, const char * str2){
    char * s1 = (char *) str1;
    char * s2 = (char *) str2;
    while (*s1 != '\0' && *s2 != '\0' && cto_lower(*s1) == cto_lower(*s2)){
        s2++;
        s1++;
    }       
    return cto_lower(*s1) - cto_lower(*s2);
}




/**
 * @brief Initializes the hash table internally used for dictionaries.
 * 
 * This must be the first function you call if you want to use the dictionary class. 
 * You don't need to call this function if you have several dictionaries. You just need to 
 * call it one time in the whole code even if you have a multi-thread application (in this
 * case, it should be a global variable).
 * @return A pointer to the generated hash table in the memory.
 */
static WORD * init_hash_table(void){
    WORD * hash_table = (WORD *) malloc(TABLE_SIZE * sizeof(WORD));
    if (NULL == hash_table){
        return NULL;
    }
    for (unsigned long int i=0;i<TABLE_SIZE -1; i++)
        hash_table[i] = i;
    shuffle(hash_table, TABLE_SIZE - 1);
    return hash_table;
}

/**
 * @brief Frees the hash table generated by #init_hash_table().
 * 
 * This must be the last function to be called.
 * 
 * @param hash_table Pointer to the hash table
 */
static void free_hash_table(WORD * hash_table){
    if (NULL == hash_table)
        return;
    else
        free(hash_table);
    return;
}


/**
 * @brief Simple shuffle function to shuffle an array with size n.
 */
static void shuffle(WORD * table, unsigned int n){
    srand(time(NULL));
    //srand(1);
    unsigned int i,j, tmp;
    for (i = n-1; i> 0; i--){
        j = rand() % (i+1);
        tmp = table[j];
        table[j] = table[i];
        table[i] = tmp;
    }
}



/**
 * @brief Initializes the Dictionary in the memory.
 * @param a pointer to the function that accept one parameter as void*
 *  
 * This function initialize a dictionary context on success.
 * The input parameter is a pointer to the function written by the 
 * user to free the `value` member of the `NODE` structure.
 *
 * since users may store any type of data structure (string, integer or custum struct)
 * in the value of the dictionary, the code must know how to free() that value to prevent
 * the memory leak. That's why you need to pass a pointer to the free function and we will
 * call this function inside cdict_free()
 *
 * If you store basic types like int, float, char or double, then you can pass NULL
 * as the parameter.
 *
 * @return A pointer to #DICT structure on success or NULL on failure
 */
cdict_ctx* cdict_init(void(*free_func)(void*), void*(*copy_func)(void*)){
    if (copy_func == NULL){
        return NULL;
    }
    WORD * hash_table = init_hash_table();
    if (NULL == hash_table)
        return NULL;
    cdict_ctx * ctx = (cdict_ctx*) malloc (sizeof(cdict_ctx));
    if (NULL == ctx)
        return NULL;
    ctx->hash_table = hash_table;
    ctx->table = (PNODE) malloc(TABLE_SIZE * sizeof(NODE));
    
    if (ctx->table == NULL){
        free(ctx);
        return NULL;
    }
    ctx->free_func = free_func;
    ctx->copy_func = copy_func;
    for (unsigned long int i=0; i< TABLE_SIZE; i++){
        ctx->table[i].key = NULL;
        ctx->table[i].value = NULL;
        ctx->table[i].next = NULL;
    }

    ctx->errmsg = (char *) malloc(sizeof(char) * 0xFF);
    ctx->err = 0;
    //let's assign functions to pointers
    return ctx;
}


/**
 * @brief This is an internal function. You should never call this function.
 * 
 * This hash function is used to hash the "key" using the hash_table.
 */
static unsigned int hashme(cdict_ctx * ctx, char * key){
    if (key == NULL){
        ctx->err =  ERROR_NULL_VALUE_FOR_KEY;
        strcpy(ctx->errmsg, "Key can not be NULL for a dictionary!");
        return 0;
    }
    unsigned long long int j;
    j=1;
    for (unsigned int i=0; i< strlen(key); ++i){
        j *= (WORD)key[i];
        j = (j % (TABLE_SIZE)) & 0xFFFF;
    }
    return ctx->hash_table[j];
}

/**
 * @brief 
 * @param dict
 * @param key
 *
 * @return A pointer to the stored value as void*. You don't need to free
 * the return value after use. Calling cdict_free() will free all the values.
 */
void * cdict_get(cdict_ctx* ctx, char * key){
    if (NULL == ctx)
        return NULL;
    if (NULL == key){
        ctx->err = ERROR_NULL_VALUE_FOR_KEY;
        strcpy(ctx->errmsg, "The key can not be null for a dictionary!");
        return NULL;
    }
    WORD index = hashme(ctx, key);
    if (ctx->err != ERROR_OK)
        return NULL;
    PNODE tmp = &(ctx->table[index]);
    if (NULL == tmp){
        return NULL;
    }
    while(1){
        if (NULL == tmp)
            return NULL;
        if (tmp->key == NULL)
            return NULL;
        if (strcmp(tmp->key, key) == 0){
            return (void*)tmp->value;
        }else{
            tmp = tmp->next;
            continue;
        }
    }
}



/**
 * @brief get a value from the dictionary by providing the key (case-insensitive key)
 * @param dict
 * @param key the key to get the value for. The comparisons of the keys are case-insensitive
 *
 * @return A pointer to the stored value as void*. You don't need to free
 * the return value after use. Calling cdict_free() will free all the values.
 */
void * cdict_get_nocase(cdict_ctx* ctx, char * key){
    if (NULL == ctx)
        return NULL;
    if (NULL == key){
        ctx->err = ERROR_NULL_VALUE_FOR_KEY;
        strcpy(ctx->errmsg, "The key can not be null for a dictionary!");
        return NULL;
    }
    // make sure you lower case the key first
    char * clone_key  = strdup(key);
    for (int i=0; clone_key[i]; i++)
        clone_key[i] = cto_lower(clone_key[i]);

    WORD index = hashme(ctx, clone_key);
    if (ctx->err != ERROR_OK){
        free(clone_key);
        return NULL;
    }
    PNODE tmp = &(ctx->table[index]);
    if (NULL == tmp){
        free(clone_key);
        return NULL;
    }
    while(1){
        if (NULL == tmp){
            free(clone_key);
            return NULL;
        }
        if (tmp->key == NULL){
            free(clone_key);
            return NULL;
        }
        if (cstr_ccmp(tmp->key, clone_key) == 0){
            free(clone_key);
            return (void*)tmp->value;
        }else{
            tmp = tmp->next;
            continue;
        }
    }
}


/**
 * @brief Sets the new value for a given key (make key lower case first).
 * @param ctx The context returned by cdict_init()
 * @param key The key for the value which we convert it to lowercase first
 * @param value The value to set for the given key
 *
 * We internally copy both key and value. So you can free the params
 * after calling cdict_set().
 *
 * @return Returns 0 on success and non-zero if fails
 */
int cdict_set_nocase(cdict_ctx *ctx, char * key, void * value){
    if (NULL == ctx)
        return ERROR_DICT_POINTER_IS_NULL;
    if (NULL == key){
        ctx->err = ERROR_NULL_VALUE_FOR_KEY;
        strcpy(ctx->errmsg, "Key can not be NULL!");
        return ERROR_NULL_VALUE_FOR_KEY;
    }
    
    char * clone_key = strdup(key);

    // this is here to make sure we make it lower case for hash table
    for (int i=0; clone_key[i]; i++)
        clone_key[i] = cto_lower(clone_key[i]);

    WORD hash_val = hashme(ctx, clone_key);
    
    void * clone_val = ctx->copy_func(value);
    
    if (NULL == ctx->table[hash_val].key){
        // no collision
        ctx->table[hash_val].key = clone_key;
        ctx->table[hash_val].value = clone_val;
        ctx->table[hash_val].next = NULL;
        return ERROR_OK;
    }else{
        // we have collision
        PNODE tmp = &(ctx->table[hash_val]);
        while(tmp->next != NULL){
            if (strcmp(tmp->key, clone_key) != 0){
                tmp = tmp->next;
                continue;
            }else{
                break;
            }
        }
        if (strcmp(tmp->key, clone_key) == 0){
            if (tmp->value != NULL)
                ctx->free_func(tmp->value);
            // we don't need the clone_key
            free(clone_key);
            tmp->value = clone_val;
            return ERROR_OK;
        }else{
            tmp->next = (PNODE) malloc(sizeof(NODE));
            if (NULL == tmp->next){
                ctx->err = CDICT_ERROR_CAN_NOT_ALLOCATE_MEMORY;
                ctx->free_func(clone_val);
                free(clone_key);
                strcpy(ctx->errmsg, "Can not allocate memory for the new key-value pair!");
                return CDICT_ERROR_CAN_NOT_ALLOCATE_MEMORY;
            }
            tmp = tmp->next;
            tmp->key = clone_key;
            tmp->value = clone_val;
            tmp->next = NULL;
            return ERROR_OK;
        }
    }
}





/**
 * @brief Sets the new value for a given key.
 * @param ctx The context returned by cdict_init()
 * @param key The key for the value
 * @param value The value to set for the given key
 *
 * We internally copy both key and value. So you can free the params
 * after calling cdict_set().
 *
 * @return Returns 0 on success and non-zero if fails
 */
int cdict_set(cdict_ctx *ctx, char * key, void * value){
    if (NULL == ctx)
        return ERROR_DICT_POINTER_IS_NULL;
    if (NULL == key){
        ctx->err = ERROR_NULL_VALUE_FOR_KEY;
        strcpy(ctx->errmsg, "Key can not be NULL!");
        return ERROR_NULL_VALUE_FOR_KEY;
    }

    WORD hash_val = hashme(ctx, key);
    
    char * clone_key = strdup(key);
    void * clone_val = ctx->copy_func(value);
    
    if (NULL == ctx->table[hash_val].key){
        // no collision
        ctx->table[hash_val].key = clone_key;
        ctx->table[hash_val].value = clone_val;
        ctx->table[hash_val].next = NULL;
        return ERROR_OK;
    }else{
        // we have collision
        PNODE tmp = &(ctx->table[hash_val]);
        while(tmp->next != NULL){
            if (strcmp(tmp->key, clone_key) != 0){
                tmp = tmp->next;
                continue;
            }else{
                break;
            }
        }
        if (strcmp(tmp->key, clone_key) == 0){
            if (tmp->value != NULL)
                ctx->free_func(tmp->value);
            // we don't need the clone_key
            free(clone_key);
            tmp->value = clone_val;
            return ERROR_OK;
        }else{
            tmp->next = (PNODE) malloc(sizeof(NODE));
            if (NULL == tmp->next){
                ctx->err = CDICT_ERROR_CAN_NOT_ALLOCATE_MEMORY;
                ctx->free_func(clone_val);
                free(clone_key);
                strcpy(ctx->errmsg, "Can not allocate memory for the new key-value pair!");
                return CDICT_ERROR_CAN_NOT_ALLOCATE_MEMORY;
            }
            tmp = tmp->next;
            tmp->key = clone_key;
            tmp->value = clone_val;
            tmp->next = NULL;
            return ERROR_OK;
        }
    }
}


/**
 * @brief Frees the memory allocated for the dictionary.
 * 
 * Call this method after using the dictionary.
 * 
 * @param dict A pointer to #DICT structure.
 */
void cdict_free(cdict_ctx * ctx){
    PNODE tmp = NULL;
    PNODE tmp_1 = NULL;
    if (NULL == ctx)
        return;
    //free the pointer to errmsg
    free(ctx->errmsg);
    // we need to empty the hash_table
    if (ctx->table != NULL){
        for (unsigned int i=0; i< TABLE_SIZE; i++){
            if (ctx->table[i].key == NULL)
                continue;
            tmp = ctx->table[i].next;
            free(ctx->table[i].key);
            ctx->free_func(ctx->table[i].value);
            while (tmp){
                tmp_1 = tmp;
                free(tmp_1->key);
                ctx->free_func(tmp_1->value);
                tmp = tmp_1->next;
                free(tmp_1);
            }
        }
    }
    free(ctx->table);
    free_hash_table(ctx->hash_table);
    free(ctx);
    return;
}

/**
 * @brief Checks if the key exists in the dictionary
 * @param dict Pointer to #DICT structure
 * @param key The key we want to check the existence
 * @return Returns 1 if key exists otherwise 0
 */
int cdict_has_key(cdict_ctx* ctx, char * key){
    if (NULL == ctx)
        return 0;
    if (NULL == key){
        ctx->err = ERROR_NULL_VALUE_FOR_KEY;
        strcpy(ctx->errmsg, "The key can not be null for a dictionary");
        return 0;
    }
    WORD index = hashme(ctx, key);
    if (ctx->err != ERROR_OK)
        return 0;
    PNODE tmp = &(ctx->table[index]);
    while(1){
        if (NULL == tmp)
            return 0;
        if (tmp->key == NULL)
            return 0;
        if (strcmp(tmp->key, key) == 0){
            return 1;
        }else{
            tmp = tmp->next;
            continue;
        }
    }
    return 0;
}


/**
 * @brief Checks if the key exists in the dictionary (case-insensitive comparison)
 * @param dict Pointer to #DICT structure
 * @param key The key we want to check the existence
 * @return Returns 1 if key exists otherwise 0
 */
int cdict_has_key_nocase(cdict_ctx* ctx, char * key){
    if (NULL == ctx)
        return 0;
    if (NULL == key){
        ctx->err = ERROR_NULL_VALUE_FOR_KEY;
        strcpy(ctx->errmsg, "The key can not be null for a dictionary");
        return 0;
    }
    WORD index = hashme(ctx, key);
    if (ctx->err != ERROR_OK)
        return 0;
    PNODE tmp = &(ctx->table[index]);
    while(1){
        if (NULL == tmp)
            return 0;
        if (tmp->key == NULL)
            return 0;
        if (cstr_ccmp(tmp->key, key) == 0){
            return 1;
        }else{
            tmp = tmp->next;
            continue;
        }
    }
    return 0;
}




/**
 * @brief This function free the keylist structure returned by cdict_keys()
 * @param klst A pointer to cdict_keylist structure
 *
 * @return Nothing
 */
void cdict_free_keylist(cdict_keylist* klst, int clone_keys){
    if (clone_keys)
        for (unsigned int i=0; i< klst->len; ++i)
            free(klst->lst[i]);
    free(klst->lst);
    free(klst);
    return;
}



/**
 * @brief Get the list of keys from the dictionary
 * @param dict The dictionary to set the key-value in
 * @param clone_keys pass 1 if you want to get a copy of the keys or 0 to return a pointer to keys
 *
 * if clone_keys is 1, then the function will copy all the internal keys using strdup().
 * This means that users must free the return cdict_keylist result by calling 
 * cdict_free_keylist() and setting the clone_keys to 1.
 * 
 * This method is good for removing key-value from a dictionary in a loop.
 *
 * @return returns a char** array (User is responsible to free the key list by calling free() function)
 */
cdict_keylist * cdict_keys(cdict_ctx* ctx, int clone_keys){
    unsigned long int cnt = 0;
    PNODE tmp = NULL;
    for (unsigned long int i=0; i< TABLE_SIZE; i++){
        if (ctx->table[i].key != NULL){
            cnt += 1;
            tmp = ctx->table[i].next;
            while(tmp && tmp->key){
                cnt += 1;
                tmp = tmp->next;
            }
        }
    }
    cdict_keylist *klst = (cdict_keylist*) malloc(sizeof(cdict_keylist));
    if (!klst){
        ctx->err = CDICT_ERROR_CAN_NOT_ALLOCATE_MEMORY;
        strcpy(ctx->errmsg, "Can not allocate memory for keys");
        return NULL;
    }
    klst->len = cnt;
    if (cnt == 0)
        return klst;
    // allocate cnt * (sizeof(char *))
    klst->lst = (char**) malloc (cnt * sizeof(char*));
    if (NULL == klst->lst){
        ctx->err = CDICT_ERROR_CAN_NOT_ALLOCATE_MEMORY;
        strcpy(ctx->errmsg, "Can not allocate memory for keys");
        free(klst);
        return NULL;
    }
    tmp = NULL;
    cnt = 0;
    ctx->err = ERROR_OK;
    for (unsigned long int i=0; i< TABLE_SIZE; ++i){
        if (ctx->table[i].key != NULL){
            klst->lst[cnt] = clone_keys?strdup(ctx->table[i].key):ctx->table[i].key;
            cnt += 1;
            tmp = ctx->table[i].next;
            while (tmp && tmp->key){
                klst->lst[cnt] = clone_keys?strdup(tmp->key):tmp->key;
                cnt += 1;
                tmp = tmp->next;
            }
        }
    }
    return klst;
}

/**
 * @brief Removes a key-value pair from the dictionary
 * @param ctx context returned by cdict_init()
 * @param key key to remove from dictionary
 *
 * @return nothing!
 */
void cdict_remove(cdict_ctx * ctx, char * key){
    if (!ctx)
        return;
    if (!key)
        return;
    PNODE tmp, tmp_1;
    WORD index = hashme(ctx, key);
    // if key is not there return
    if (!cdict_has_key(ctx, key))
        return;
    // key is there
    tmp = &(ctx->table[index]);
    if (strcmp(key, tmp->key) == 0){
        // in the main chain
        if (tmp->next == NULL){
            free(tmp->key);
            tmp->key = NULL;
            ctx->free_func(tmp->value);
            tmp->value = NULL;
            return;
        }else{
            // with link
            free(tmp->key);
            ctx->free_func(tmp->value);
            tmp->key = strdup(tmp->next->key);
            tmp->value = ctx->copy_func(tmp->next->value);
            tmp_1 = tmp->next;
            tmp->next = tmp_1->next;
            // now remove tmp_1
            free(tmp_1->key);
            ctx->free_func(tmp_1->value);
            free(tmp_1);
            return;
        }
    }
    // not in the main chain
    tmp_1 = tmp;
    tmp = tmp->next;
    while (tmp){
        if (strcmp(tmp->key, key) == 0){
            break;
        }
        tmp_1 = tmp;
        tmp = tmp->next;
    }
    // tmp is the candidate, tmp_1 is its parent
    if (tmp->next == NULL){
        // it's a leaf
        free(tmp->key);
        ctx->free_func(tmp->value);
        free(tmp);
        tmp_1->next = NULL;
        return;
    }else{
        // it's in the middle
        tmp_1->next = tmp->next;
        free(tmp->key);
        ctx->free_func(tmp->value);
        free(tmp);
        return;
    }
    return;
}

