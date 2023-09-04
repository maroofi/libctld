/** @file */
#include <cdict.h>

#define CTLD_ERROR_MALLOC_FAILED 1
#define CTLD_CONTEXT_INIT_FAILED 2
#define CTLD_OPEN_FILE_FAILED 3
#define CTLD_READ_FILE_FAILED 4
#define CTLD_LIST_SPLIT_FAILED 5
#define CTLD_PARSE_LIST_FAILED 6
#define CTLD_NO_MATCH_FOUND 7

/**
 * @details This is an internal structure for each entry of PSL data.
 */
struct ctld_node{
    char * name;            ///< the suffix name in PSL
    int is_private;         ///< 1 if the suffix is a private suffix else 0
    int has_priority;       ///< 1 if the suffix has priority else 0
};



/**
 * @details This structure is returned as a result of calling ctld_parse() function
 * 
 */
struct ctld_result{
    char * suffix;                  ///< TLD or suffix of the domain name
    char * registered_domain;       ///< Registered domain based on the suffix
    char * domain;                  ///< domain name part of the registered domain(without suffix)
    char * fqdn;                    ///< fully-qualified domain name of the given record
};


/**
* @details Type definition of the struct ctld_result
*/
typedef struct ctld_result ctld_result;



/**
* @details Type definition of the struct ctld_node
*/
typedef struct ctld_node ctld_node;



/**
 * @details This structure is the main context of libctld library.
 * The structure returns by either calling ctld_parse_file() or
 * by calling ctld_parse_string()
 * 
 */
struct ctld_ctx{
    cdict_ctx * list_private;           ///< contains the private part of the suffix list
    cdict_ctx * list_public;            ///< contains the public part of the suffix list
    int errcode;                        ///< any possible error code returned by library
};

/**
* @details Type definition of the struct ctld_ctx
*/
typedef struct ctld_ctx ctld_ctx;


void ctld_print_error(ctld_ctx * ctx);
ctld_ctx * ctld_parse_string(char * data);
void ctld_result_free(ctld_result*);
int ctld_is_domain_valid(char * domain);
void ctld_free(ctld_ctx*);
ctld_ctx * ctld_parse_file(char * filename);
ctld_result * ctld_parse(ctld_ctx * ctx, char * domain, int use_private_suffix);
int ctld_add_custom_suffix(ctld_ctx * ctx, char * suffix);
