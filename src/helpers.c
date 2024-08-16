#include "helpers.h"

void debugPrint(const char *file, int line, const char *format, ...){
    va_list args;
    va_start(args, format);
    printf("[%s:%i] ", file, line);
    vprintf(format, args);
    va_end(args);
}

#ifdef FUNCTION_CHECKER
#include "piano.h"
#include "interface.h"
extern struct Interface *interface;

void checkFunction(void *thisFunc, void *callSite){
    if(interface != NULL && interface->piano != NULL && interface->piano->blackKeysTrigCount != 0){
    // if(interface != NULL && interface->piano != NULL){
        fprintf(stderr, "variable was changed inside %p %p\n", thisFunc, callSite);
        exit(1);
    }
}

void __cyg_profile_func_enter(void *thisFunc, void *callSite){
    checkFunction(thisFunc, callSite);
    // fprintf(stderr, "inside function %zu\n");
}

void __cyg_profile_func_exit(void *thisFunc, void *callSite){
    checkFunction(thisFunc, callSite);
    // fprintf(stderr, "outside of function %zu\n");
}
#endif
