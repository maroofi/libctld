/// @file libctld.c
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cstrlib.h>
#include <cdict.h>
#include <clist.h>
#include <libctld.h>
#include <idn2.h>


/****************Static declaration********************/
static void ctld_node_free(void* node);
static void * ctld_node_copy(void* node);
static ctld_ctx * ctld_init(void);
static ctld_result * ctld_choose_best(PCLIST all_match, char * domain);
static int cstr_ccmp(const char * str1, const char * str2);
static int cto_lower(int c);
static char * ctld_read_file(char * filename);

static int ctld_parse_list(char * data, cdict_ctx* list_public, cdict_ctx* list_private);

static void ctld_node_free(void* node){
    ctld_node *to_free = (ctld_node*) node;
    free(to_free->name);
    free(to_free);
    return;
}

static void * ctld_node_copy(void* node){
    return node;
}


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

static ctld_ctx * ctld_init(void){
    ctld_ctx * ctx = (ctld_ctx*) malloc(sizeof(ctld_ctx));
    if (!ctx){
        return NULL;
    }
    ctx->errcode = 0;
    ctx->list_public = cdict_init(ctld_node_free, ctld_node_copy);
    if (!ctx->list_public){
        free(ctx);
        return NULL;
    }
    ctx->list_private = cdict_init(ctld_node_free, ctld_node_copy);
    if (!ctx->list_private){
        cdict_free(ctx->list_public);
        free(ctx);
        return NULL;
    }
    return ctx;
}


static ctld_node * ctld_search_in_list(cdict_ctx * lst, char * name){
    // looking for a name in lst which is a dict
    return cdict_get_nocase(lst, name);
}

static unsigned int scstr_count(const char * str, int ch){
    // counts the number of occurrence of ch in str
    // str must not be null
    int cnt = 0;
    char * tmp = (char*)str;
    while (*tmp){
        if (*tmp == ch) 
            cnt++;
        tmp++;
    }   
    return cnt;
}

static char * ctld_read_file(char * filename){
    FILE * f = fopen(filename, "r");
    if (!f){
        perror("ERROR");
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char * data = (char*) malloc(size+1);
    if (!data){
#ifdef DEBUG
        fprintf(stderr, "ERROR: malloc() failed\n");
#endif
        return NULL;
    }
    size_t read = fread(data, 1, size, f);
    if (read != size){
#ifdef DEBUG
        fprintf(stderr, "Can not read the file...\n");
#endif
        return NULL;
    }
    data[size] = '\0';
    fclose(f);
    return data;
}


static ctld_result * ctld_choose_best(PCLIST all_match, char * domain){
    ctld_node * best = NULL;
    ctld_node * item = NULL;
    unsigned long int level = 0;
    unsigned long int tmp_level = 0;
    unsigned long int domain_len = strlen(domain);
    unsigned long int tmp_len = 0;
    for (int i=0; i<= all_match->last_item; ++i){
        item = (ctld_node *)all_match->list[i];
        tmp_len = strlen(item->name);
        tmp_level = scstr_count(item->name, '.') + 1;
        if (item->has_priority){   // get the one with the priority
            best = item;
            break;
        }
        if ( tmp_level> level || best == NULL){
            if (item->name[0] == '*')   // we don't match as * will be empty
                if (domain_len <= tmp_len - 2)
                    continue;
            level = tmp_level;
            best = item;
            continue;
        }
    }
    if (!best || !best->name)
        return NULL;
    // we have one candidate
    char * tmp = NULL;
    tmp = best->name[0] == '*'?strdup(best->name + 2):strdup(best->name);
    ctld_result * result = (ctld_result*) malloc(sizeof(ctld_result));
    if (!result){
        free(tmp);
        return NULL;
    }
    result->suffix = NULL;
    result->registered_domain = NULL;
    result->fqdn = NULL;
    result->domain = NULL;
    PSTR str = str_init(tmp);
    free(tmp);
    PSTR str_domain = str_init(domain);
    PSPLITLIST str_split = NULL;
    int max_split;
    // make sure to split based on dot but not the suffix part
    max_split = str_domain->str_count(str_domain, ".") - str->str_count(str, ".");
    str_split = str_domain->str_split(str_domain, ".", max_split);
    if (best->has_priority){
        // first part is domain name and the rest is tld
        PSPLITLIST  str_tmp_split = str->str_split(str, ".", 1);
        result->suffix = strdup(str_tmp_split->list[1]);
        str->str_setval(str, result->suffix);
        str_free_splitlist(str_tmp_split);
    }else if (best->name[0] == '*'){
        str->str_prepend_char(str, '.');
        str->str_prepend_string(str, str_split->list[str_split->len -2]);
        result->suffix = str->str_copy(str);
    }else{
        // suffix is exactly what it is
        result->suffix = str->str_copy(str);
    }
    // now the suffix might be different and we need to re-split
    str_free_splitlist(str_split);
    max_split =  str_domain->str_count(str_domain, ".") - str->str_count(str, ".");
    str_split = str_domain->str_split(str_domain, ".", max_split);

    result->domain = str_split->len <= 1?NULL:strdup(str_split->list[str_split->len -2]);
    unsigned long int len_domain = 0, len_suffix = 0;
    len_suffix = str->len;
    str_free_splitlist(str_split);

    str_free(str);
    if (result->domain){
        len_domain = strlen(result->domain);
        result->registered_domain = (char*) malloc(len_domain + len_suffix + 2);
    }
    if (result->registered_domain){
        memcpy(result->registered_domain, result->domain, len_domain);
        memcpy(result->registered_domain + len_domain, (const char*)".", 1);
        memcpy(result->registered_domain + len_domain + 1, result->suffix, len_suffix);
        result->registered_domain[len_domain + len_suffix + 1] = '\0';
    }
    str_free(str_domain);
    result->fqdn = result->domain?strdup(domain):NULL;
    return result;
}

static int ctld_parse_list(char * data, cdict_ctx* list_public, cdict_ctx* list_private){
    if (!data){
#ifdef DEBUG
        fprintf(stderr, "Can not get the data from file...\n");
#endif
        return 1;
    }
    PSTR str = str_init(NULL);
    str->str_setval(str, data);
    PSPLITLIST plst = str->str_split(str, "\n", -1);
    if (!plst){
#ifdef DEBUG
        fprintf(stdout, "Can not split the list\n");
#endif
        return 1;
    }
    PSTR line = str_init(NULL);
    str_free(str);
    char * stripped_line = NULL;
    int start_icann_part = 0;
    int end_icann_part = 0;
    int start_private_part = 0;
    int end_private_part = 0;
    char * tmp_str = NULL;
    for (int i=0; i< plst->len; ++i){
        line->str_setval(line, plst->list[i]);
        stripped_line = line->str_rstrip(line, NULL);
        line->str_setval(line, stripped_line);
        free(stripped_line);
        if (line->str_find(line, "===BEGIN ICANN DOMAINS===") != -1)
            start_icann_part = 1;
        if (line->str_find(line, "===END ICANN DOMAINS===") != -1)
            end_icann_part = 1;
        if (line->str_find(line, "===BEGIN PRIVATE DOMAINS===") != -1)
            start_private_part = 1;
        if (line->str_find(line, "===END PRIVATE DOMAINS===") != -1)
            end_private_part = 1;
        if (line->str_startswith(line, "//"))
            continue;
        if (strcmp(line->str, "") == 0)
            continue;
        // print only public list
        if (start_private_part == 1 && end_private_part == 0){
            struct ctld_node * new_node = (struct ctld_node*) malloc(sizeof(struct ctld_node));
            if (!new_node){
#ifdef DEBUG
                fprintf(stderr, "ERROR: malloc() failed\n");
#endif
                continue;
            }
            new_node->is_private = 1;
            new_node->has_priority = line->len && line->str[0] == '!'?1:0;
            tmp_str = new_node->has_priority?line->str_copy(line):NULL;
            new_node->name = tmp_str?strdup(tmp_str+1):line->str_copy(line);
            cdict_set(list_private, new_node->name, (void*) new_node);
            free(tmp_str);
            // if we have IDNA, add another node to the list
            int idna_result;
            char * idna_out = NULL;
            if ((idna_result = idna_to_ascii_8z(new_node->name, &idna_out, IDN2_NONTRANSITIONAL)) == 0){
                if (idna_out && cstr_ccmp(idna_out, new_node->name) != 0){
                    // here we have IDNA, we need to add a new node for this
                    struct ctld_node * new_node_idna = (struct ctld_node*) malloc(sizeof(struct ctld_node));
                    if (!new_node_idna){
#ifdef DEBUG
                        fprintf(stderr, "ERROR: malloc() failed\n");
#endif  
                        free(idna_out);
                        continue;
                    }
                    new_node_idna->is_private = 1;
                    new_node_idna->has_priority = line->len && line->str[0] == '!'?1:0;
                    new_node_idna->name = strdup(idna_out);
                    cdict_set(list_private, new_node_idna->name, (void*) new_node_idna);
                    free(idna_out);
                }else{
                    if (idna_out){
                        free(idna_out);
                        idna_out = NULL;
                    }
                }
            }
            continue;
        }
        if(start_icann_part == 1 && end_icann_part == 0){
            struct ctld_node * new_node = (struct ctld_node*) malloc(sizeof(struct ctld_node));
            if (!new_node){
#ifdef DEBUG
                fprintf(stderr, "ERROR: malloc() failed\n");
#endif
                continue;
            }
            new_node->is_private = 0;
            new_node->has_priority = line->len && line->str[0] == '!'?1:0;
            tmp_str = new_node->has_priority?line->str_copy(line):NULL;
            new_node->name = tmp_str?strdup(tmp_str+1):line->str_copy(line);
            free(tmp_str);
            cdict_set(list_public, new_node->name, (void*)new_node);
            // if we have IDNA, add another node to the list
            int idna_result;
            char * idna_out = NULL;
            if ((idna_result = idna_to_ascii_8z(new_node->name, &idna_out, IDN2_NONTRANSITIONAL)) == 0){
                if (idna_out && cstr_ccmp(idna_out, new_node->name) != 0){
                    // here we have IDNA, we need to add a new node for this
                    struct ctld_node * new_node_idna = (struct ctld_node*) malloc(sizeof(struct ctld_node));
                    if (!new_node_idna){
#ifdef DEBUG
                        fprintf(stderr, "ERROR: malloc() failed\n");
#endif
                        free(idna_out);
                        continue;
                    }
                    new_node_idna->is_private = 0;
                    new_node_idna->has_priority = line->len && line->str[0] == '!'?1:0;
                    new_node_idna->name = strdup(idna_out);
                    cdict_set(list_public, new_node_idna->name, (void*) new_node_idna);
                    free(idna_out);
                }else{
                    if (idna_out){
                        free(idna_out);
                        idna_out = NULL;
                    }
                }
            }
            continue;
        }
    }
    str_free_splitlist(plst);
    str_free(line);
    return 0;
}



/**
 * @brief free the memory used by ctld_result structure
 * 
 * This should be called by users to free the allocated memory.
 *
 * @param res the return result of the ctld_parse() API
 * @return Nothing
 */
void ctld_result_free(ctld_result* res){
    if (!res)
        return;
    free(res->domain);
    free(res->fqdn);
    free(res->registered_domain);
    free(res->suffix);
    free(res);
    return;
}


/**
 * @brief free the memory used by ctld context
 * 
 * This should be called by users to free the allocated memory.
 *
 * @param ctx the context created by ctld_parse_file()
 * @return Nothing
 */
void ctld_free(ctld_ctx * ctx){
    if (!ctx)
        return;
    if (ctx->list_public)
        cdict_free(ctx->list_public);
    if (ctx->list_private)
        cdict_free(ctx->list_private);
    free(ctx);
    return;
}


/**
 * @brief parse the PSL data from a null-terminated memory.
 * 
 * the memory should be free by users if it needs to be freed.
 *
 * @param data A pointer to a null-terminated string containing PSL data.
 * @return a pointer to the ctld context which can be used in ctld_parse()
 */
ctld_ctx * ctld_parse_string(char * data){
    ctld_ctx * ctx = ctld_init();
    if (!ctx){
        return NULL;
    }
    ctx->errcode = 0;
    if (ctld_parse_list(data, ctx->list_public, ctx->list_private)){
#ifdef DEBUG
        fprintf(stderr, "Something is wrong in parsing list...\n");
#endif
        return NULL;
    }
    // now we have list_private and list_public
    // print 10 public
    return ctx;
}



/**
 * @brief parse PSL data by passing the filename.
 * 
 * @param filename A pointer to a null-terminated string contains the name of the file.
 * @return a pointer to the ctld context which can be used in ctld_parse()
 */
ctld_ctx * ctld_parse_file(char * filename){
    char * data = ctld_read_file(filename);
    ctld_ctx * ctx = ctld_init();
    if (!ctx){
        return NULL;
    }
    ctx->errcode = 0;
    if (ctld_parse_list(data, ctx->list_public, ctx->list_private)){
#ifdef DEBUG
        fprintf(stdout, "Something is wrong in parsing list...\n");
#endif
        return NULL;
    }
    free(data);
    // now we have list_private and list_public
    // print 10 public
    return ctx;
}


/**
 * @brief check if the domain name contains illegal character
 * 
 * One needs to be very careful to use this function. some companies have illegal
 * characters like '_' in the subdomain part of the domain name. So only use this
 * function if you know what you are doing.
 *
 * @param domain A pointer to a null-terminated string (domain name)
 * @return 1 on success and 0 on failure
 */
int ctld_is_domain_valid(char * domain){
    // simple checks to see if the fully-qualified domain is valid
    // the domain must have atleast one dot
    // at least 4 characters
    // no special character
    // returns 1 if valid 0 if invalid
    if (!domain)
        return 0;
    PSTR str = str_init(domain);
    if (str->str_len(str) < 4){
        str_free(str);
        return 0;
    }
    if (str->str_find(str, ".") == -1){
        str_free(str);
        return 0;
    }
    if (str->str_any_of_in(str, "!@#$%^&*()_+=\\/,|'\"")){
        str_free(str);
        return 0;
    }
    str_free(str);
    return 1;
}


/**
 * @brief parse the passed domain name based on the PSL and returns the result structure.
 *
 * @param ctx context created by calling ctld_parse_file() or ctld_parse_string()
 * @param domain the domain name you want to parse
 * @param use_private_suffix 0 means do not use private part of the PSL and 1 means 
 * using the private part of the PSL.
 * 
 * This is the main API of the library.
 *
 * @return Returns an instance of ctld_result on success and NULL on failure.
 */
ctld_result * ctld_parse(ctld_ctx * ctx, char * domain, int use_private_suffix){
    if (!domain || !ctx)
        return NULL;
    // do we really care if the fqdn is correct or not?
    //if (ctld_is_domain_valid(domain) != 1)
    //    return NULL;
    ctld_node * match_public, *match_private = NULL;
    PSTR str = str_init(domain);
    PSTR new_str = str_init(NULL);
    // just search for a perfect match in list
    PCLIST all_match = clist_init(ctld_node_free);

    match_public = ctld_search_in_list(ctx->list_public, domain);
    match_private = use_private_suffix?ctld_search_in_list(ctx->list_private, domain):NULL;
    if (match_public)
        clist_add_item(all_match, (void*)match_public);    
    if (match_private && use_private_suffix)
        clist_add_item(all_match, (void*) match_private); 
    new_str->str_setval(new_str, "*.");
    new_str->str_append_string(new_str, domain);
    match_public = ctld_search_in_list(ctx->list_public, new_str->str);
    match_private = use_private_suffix?ctld_search_in_list(ctx->list_private, new_str->str):NULL;
    if (match_public)
        clist_add_item(all_match, (void*) match_public);
    if (match_private && use_private_suffix)
        clist_add_item(all_match, (void*)match_private);    

    PSPLITLIST str_split = str->str_split(str, ".", -1);
    SPLITLIST new_split;
    for (int i=1; i<str_split->len && str_split->len > 1; ++i){
        new_split.len = str_split->len -i;
        new_split.list = str_split->list + i;
        new_str->str_join(new_str, &new_split, ".");
        match_public = ctld_search_in_list(ctx->list_public, new_str->str);
        match_private = use_private_suffix?ctld_search_in_list(ctx->list_private, new_str->str):NULL;
        if (match_public)
            clist_add_item(all_match, (void*) match_public);
        if (use_private_suffix && match_private)
            clist_add_item(all_match, (void*)match_private);    

        new_str->str_prepend_string(new_str, "*.");
        match_public = ctld_search_in_list(ctx->list_public, new_str->str);
        match_private = use_private_suffix?ctld_search_in_list(ctx->list_private, new_str->str):NULL;
        if (match_public)
            clist_add_item(all_match, (void*) match_public);
        if (use_private_suffix && match_private)
            clist_add_item(all_match, (void*)match_private);    
        
    }
    
    str_free_splitlist(str_split);
    str_free(new_str);
    str_free(str);
    if (!all_match || all_match->first_empty == 0){
        free(all_match->list);
        free(all_match);
        ctx->errcode = CTLD_NO_MATCH_FOUND;
#ifdef DEBUG
        fprintf(stdout, "Can not find a match for a given domain: %s\n", domain);
#endif
        return NULL;
    }
    ctld_result * result = ctld_choose_best(all_match, domain);
    // we can not call clist_free() on all match as
    // we have refernce not copy of the data
    // we just need to free the list and the structure itself
    // not the content inside the list
    free(all_match->list);
    free(all_match);
    return result;   
}

