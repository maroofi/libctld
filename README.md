# LIBTLD

C Library to parse public suffix list (PSL)


### PSL data

Before compiling the library, you must update PSL data (psl.dat file)
by downloading the latest file from [here](https://publicsuffix.org/list/)


### Compile and test
```bash
## you must have libidn2 installed before compiling

# will make the library and binary files in ./bin folder
make

# ro run the test
make test

# build the doc
doxygen Doxyfile
```

## How to use
```c
#include <libctld.h>
#include <stdio.h>

// can manually compile it with:
// gcc -o example example.c -I./include src/cdict.c src/cstrlib.c src/clist.c src/libctld.c -lidn2

int main(int argc, char ** argv){
    char * psl_data = "psl.dat";

    ctld_ctx * ctx = ctld_parse_file(psl_data);
    if (!ctx){
        fprintf(stderr, "Can not create CTLD context\n");
        return 1;
    }

    ctld_result * res = ctld_parse(ctx, "account.google.com", 0);
    printf("suffix: %s\n", res->suffix);        // com
    printf("registered domain: %s\n", res->registered_domain);  // google.com
    printf("fqdn: %s\n", res->fqdn);    // account.google.com
    printf("domain: %s\n", res->domain);    // google
    ctld_result_free(res);
    ctld_free(ctx);
    return 0;
}
```

### Exposed API

- void ctld\_result\_free(ctld\_result *res)
 
- void ctld\_free(ctld\_ctx *ctx)
 
- ctld\_ctx * ctld\_parse\_string(char *data)
 
- ctld\_ctx * ctld\_parse\_file(char *filename)
 
- int ctld\_is\_domain\_valid(char *domain)
 
- ctld_result * ctld_parse(ctld\_ctx *ctx, char *domain, int use\_private\_suffix)




