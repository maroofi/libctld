/* Author: Sourena MAROOFI */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifndef UNITTEST_H 
#define UNITTEST_H

#define ASSERT_EQ_INT(X,Y)  assert_int_equal(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" == "#Y)
#define ASSERT_GE_INT(X,Y)  assert_int_greater_equal(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" >= "#Y)
#define ASSERT_LE_INT(X,Y)  assert_int_less_equal(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" <= "#Y)
#define ASSERT_LT_INT(X,Y)  assert_int_less_than(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" < "#Y)
#define ASSERT_GT_INT(X,Y)  assert_int_greater_than(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" > "#Y)

#define ASSERT_EQ_STR(X,Y)  assert_string_equal(X,Y,__FILE__,__LINE__,__FUNCTION__, #X" == "#Y)
#define ASSERT_EQ_STR_NOCASE(X,Y) assert_string_equal_nocase(X,Y,__FILE__,__LINE__,__FUNCTION__, #X" == "#Y)
#define ASSERT_EQ_DOUBLE(X,Y) assert_double_equal(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" == "#Y)
#define ASSERT_GE_DOUBLE(X,Y) assert_double_greater_equal(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" >= "#Y)
#define ASSERT_LE_DOUBLE(X,Y) assert_double_less_equal(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" <= "#Y)
#define ASSERT_LT_DOUBLE(X,Y) assert_double_less_than(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" < "#Y)
#define ASSERT_GT_DOUBLE(X,Y) assert_double_greater_than(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" > "#Y)

#define ASSERT_EQ_FLOAT(X,Y) assert_double_equal(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" == "#Y)
#define ASSERT_GE_FLOAT(X,Y) assert_double_greater_equal(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" >= "#Y)
#define ASSERT_LE_FLOAT(X,Y) assert_double_less_equal(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" <= "#Y)
#define ASSERT_GT_FLOAT(X,Y) assert_double_greater_than(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" > "#Y)
#define ASSERT_LT_FLOAT(X,Y) assert_double_less_than(X,Y,__FILE__, __LINE__, __FUNCTION__, #X" < "#Y)

#define ASSERT_NULL(X) assert_pointer_is_null(X, __FILE__, __LINE__, __FUNCTION__, "IS_NULL("#X")")
#define ASSERT_NE_NULL(X) assert_pointer_is_not_null(X, __FILE__, __LINE__, __FUNCTION__, "IS_NOT_NULL("#X")")



#define ABORT_ON_FAIL(X) _abort_on_fail(X)

//global variable to check abort on fail
unsigned short int abort_on_fail = 1; // default is to abort

/*calling this method with 1 will abort after fail assertion 
 * and calling with 0 will continue the code but only print all the failed tests*/
int _abort_on_fail(int should_abort){
    abort_on_fail = should_abort;
    return 0;
}

char * print_error_info(const char * file_name, int line_number, const char * funcname){
    size_t len_file_name = strlen(file_name);
    len_file_name += strlen(funcname);
    // also 10 chars for line_number
    len_file_name += 10;
    char * err_str = "(line: %d, file: %s, function: %s)";
    len_file_name += strlen(err_str);
    char * err = (char *)calloc(len_file_name + 1, 1);
    if (err == NULL){
        printf("Can not allocate memory!\n");
        return NULL;
    }
    strcpy(err, err_str);
    sprintf(err,err_str, line_number, file_name, funcname);
    return err;
}


/*assert if pointer x is not NULL*/
unsigned int assert_pointer_is_not_null(void * p, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (p != NULL){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}

/*assert if pointer x is NULL*/
unsigned int assert_pointer_is_null(void * p, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (p == NULL){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}


/* assert if x == y*/
unsigned int assert_float_equal(float x, float y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x == y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}

/* assert if x > y*/
unsigned int assert_float_greater_than(float x, float y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x > y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}
/* assert if x >= y*/
unsigned int assert_float_greater_equal(float x, float y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x >= y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}
/* assert if x < y*/
unsigned int assert_float_less_than(float x, float y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x < y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}
/* assert if x <= y*/
unsigned int assert_float_less_equal(float x, float y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x <= y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}
/* assert if x == y*/
unsigned int assert_double_equal(double x, double y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x == y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}

/* assert if x > y*/
unsigned int assert_double_greater_than(double x, double y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x > y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}


/* assert if x >= y*/
unsigned int assert_double_greater_equal(double x, double y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x >= y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}



/* assert if x < y*/
unsigned int assert_double_less_than(double x, double y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x < y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}

/* assert if x <= y*/
unsigned int assert_double_less_equal(double x, double y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x <= y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}

/* assert if x > y*/
unsigned int assert_int_greater_than(int x, int y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x > y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}

/* assert if x >= y*/
unsigned int assert_int_greater_equal(int x, int y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x >= y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}


/* assert if x <= y*/
unsigned int assert_int_less_equal(int x, int y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x <= y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}


/* assert if x < y*/
unsigned int assert_int_less_than(int x, int y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x < y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}

/* assert if x == y*/
unsigned int assert_int_equal(int x, int y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (x == y){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}


/* assert if x == y (nocase)*/
unsigned int assert_string_equal_nocase(char * x, char * y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (NULL == x || NULL == y){
        char * err = print_error_info(file_name, line_number, func_name);
        printf("Test failed! (%s) %s\n", assertval, err);
        printf("%s <-----> %s %s\n", assertval, x, y);
        free(err);
        if (abort_on_fail){abort();}
        return 1;
    }
    if (strcasecmp(x,y) == 0){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    printf("%s <-----> %s %s\n", assertval, x, y);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}



/* assert if x == y*/
unsigned int assert_string_equal(char * x, char * y, const char * file_name, int line_number, const char * func_name, char * assertval){
    if (NULL == x || NULL == y){
        char * err = print_error_info(file_name, line_number, func_name);
        printf("Test failed! (%s) %s\n", assertval, err);
        free(err);
        if (abort_on_fail){abort();}
        return 1;
    }
    if (strcmp(x,y) == 0){
        return 0;   //success
    }
    char * err = print_error_info(file_name, line_number, func_name);
    printf("Test failed! (%s) %s\n", assertval, err);
    free(err);
    if (abort_on_fail){abort();}
    return 1;
}

#endif
