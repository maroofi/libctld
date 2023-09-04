#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cstrlib.h>
#include <libctld.h>
#include <url_parser.h>
#include <cmdparser.h>
#include "psl_data.h"
#include <idn2.h>


#define ISSTREQ(a,b)  (cstr_cmp(a,b)==0)
#define BSETOPT(a,b,c) do{if(ISSTREQ(a,b)){c=1;}}while(0)

#define CTLD_VERSION "0.1"
#define PRINTIFSET(x) do{if(x){printf("%s", x);}}while(0)



int main(int argc, char ** argv){
    // maybe replace it with getopt which is standard?
    ARG_CMDLINE cmd;
    cmd.accept_file = 1;
    cmd.extra = NULL;
    cmd.summary = "cTLD: Top Level Domain parser based on Public Suffix List (psl).";
    ARG_CMD_OPTION cmd_option[] = {
        {.short_option=0, .long_option = "tld", .has_param = NO_PARAM, .help="Print suffix", .tag="print_tld"},
        {.short_option=0, .long_option = "rd", .has_param = NO_PARAM, .help="Print registered domain", .tag="print_rd"},
        {.short_option=0, .long_option = "fqdn", .has_param = NO_PARAM, .help="Print fully-qualified-domain-name", .tag="print_fqdn"},
        {.short_option=0, .long_option = "private", .has_param = NO_PARAM, .help="Use private suffix list as well", .tag="use_private"},
        {.short_option=0, .long_option = "err", .has_param = NO_PARAM, .help="Print Errors only", .tag="print_err"},
        {.short_option=0, .long_option = "custom", .has_param = HAS_PARAM, .help="Add a comma-separated list of custom suffixes (no space)", .tag="custom_suffix"},
        {.short_option='h', .long_option = "help", .has_param = NO_PARAM, .help="Print this help message", .tag="print_help"},
        {.short_option='v', .long_option = "version", .has_param = NO_PARAM, .help="Print suffix", .tag="print_version"},
        {.short_option=0, .long_option = "", .has_param = NO_PARAM, .help="", .tag=NULL}
    };
    cmd.cmd_option = cmd_option;
    int err_arg = 0;
    PARG_PARSED_ARGS pargs = arg_parse_arguments(&cmd, argc, argv, &err_arg);
    if (pargs == NULL || err_arg != 0){
        arg_show_help(&cmd, argc, argv);
        return 1;
    }
    int print_tld, print_rd, print_fqdn, use_private, print_err;
    if (arg_is_tag_set(pargs, "print_help")){
        arg_show_help(&cmd, argc, argv);
        return 0;
    }
    if (arg_is_tag_set(pargs, "print_version")){
        fprintf(stdout, "cTLD v %s (maroofi@gmail.com)\n", CTLD_VERSION);
        return 0;
    }
    print_tld = arg_is_tag_set(pargs, "print_tld")?1:0;
    print_rd = arg_is_tag_set(pargs, "print_rd")?1:0;
    print_fqdn = arg_is_tag_set(pargs, "print_fqdn")?1:0;
    use_private = arg_is_tag_set(pargs, "use_private")?1:0;
    print_err = arg_is_tag_set(pargs, "print_err")?1:0;
    char * filename = cmd.extra?cmd.extra:NULL;
    char * custom_suffix = NULL;
    if (arg_is_tag_set(pargs, "custom_suffix")){
        custom_suffix = strdup(arg_get_tag_value(pargs, "custom_suffix"));
    }
    // we don't need pargs anymore, we can free the memory
    // just make valgrind shutup
    arg_free(pargs);

    FILE * fp = NULL;
    if (filename){
        fp = fopen(filename, "r");
    }else{
        fp = stdin;
    }
    if (!fp){
        perror("Error opening input file");
        return 1;
    }
    // set this as the default option
    if (print_rd == 0 && print_tld == 0 && print_fqdn == 0)
        print_rd = 1;

    char * l = NULL;
    char * psl_data = (char*)malloc(psl_dat_len + 1);
    if (!psl_data){
        fprintf(stderr, "ERROR: malloc() failed!\n");
        return 1;
    }
    memcpy(psl_data, psl_dat, psl_dat_len);
    psl_data[psl_dat_len] = '\0';
    ctld_ctx * ctx = ctld_parse_string((char*)psl_dat);
    free(psl_data);
    if (!ctx){
        fprintf(stderr, "Can not create the context for public suffix list!\n");
        return 2;
    }
    // add custom suffix if any
    if (custom_suffix){
        PSTR str = str_init(custom_suffix);
        free(custom_suffix);
        if (str){
            char * stripped = NULL;
            PSPLITLIST splt = str->str_split(str, ",", -1);
            for (int i=0; i< splt->len; ++i){
                str->str_setval(str, splt->list[i]);
                stripped = str->str_strip(str, NULL);
                if (stripped){
                    ctld_add_custom_suffix(ctx, stripped);
                    free(stripped);
                    stripped = NULL;
                }
            }
            str_free(str);
            str_free_splitlist(splt);
        }
    }
    size_t m = 0;
    size_t n = 0;
    ctld_result * result = NULL;
    struct parsed_url * purl = NULL;
    int include_private = use_private;
    char * idn_out;
    int idn_result = 0;
    char * input_buffer = NULL;
    int should_encode = 0;
    while ((n = getline(&l, &m, fp)) != -1){
        idn_result = 0;
        should_encode = 0;
        if (n == 0){
            continue;
        }
        if (n > 0 && l[n -1] == '\n')
            l[n -1] = '\0';
        if (l[0] == '\0'){
            continue;
        }
        purl = parse_url(l);
        if (purl){
            input_buffer = purl->host;
            n = 0;
            while(input_buffer[n]){
                if (input_buffer[n++] > 127){
                    should_encode = 1;
                    break;
                }
            }
            if (should_encode){
                idn_result = idn2_to_ascii_8z(input_buffer, &idn_out, IDN2_NONTRANSITIONAL);
                input_buffer = idn_out;
            }else{
                input_buffer = l;
            }
            if (idn_result == IDN2_OK){
                result = ctld_parse(ctx, input_buffer, include_private);
                parsed_url_free(purl);
                if (should_encode)
                    free(input_buffer);
            }else{
                if (print_err){
                    fprintf(stderr, "ERROR: Can not parse IDN domain: %s\n", purl->host);
                    parsed_url_free(purl);
                    continue;
                }
            }
        }else{
            n = 0;
            while(l[n]){
                if ((unsigned char)l[n++] > 127){
                    should_encode = 1;
                    break;
                }
            }
            if (should_encode){
                idn_result = idn2_to_ascii_8z(l, &idn_out, IDN2_NONTRANSITIONAL);
                input_buffer = idn_out;
            }else{
                input_buffer = l;
            }
            if (idn_result == IDN2_OK){
                result = ctld_parse(ctx, input_buffer, include_private);
                if (should_encode)
                    free(input_buffer);
            }else{
                if (print_err){
                    fprintf(stderr, "ERROR: Can not parse IDN domain: %s\n", input_buffer);
                    continue;
                }
            }
        }
        if (!result && print_err){
            fprintf(stderr, "ERROR: Can not parse: %s\n", l);
            free(l);
            continue;
        }else if (!result && !print_err){
            free(l);
            continue;
        }
        int p = 0;
        if (!print_err){
            if (print_rd){
                PRINTIFSET(result->registered_domain);
                p++;
            }
            if (print_fqdn){
                if (p)
                    printf("\t");
                p++;
                PRINTIFSET(result->fqdn);
            }
            if (print_tld){
                if (p)
                    printf("\t");
                PRINTIFSET(result->suffix);
                p++;
            }
            printf("\n");
        }
        ctld_result_free(result);
        free(l);
        l = NULL;
    }
    free(l);
    ctld_free(ctx);
    fclose(fp);
}
