#include "helpers.h"

void debugPrint(const char *file, int line, const char *format, ...){
    va_list args;
    va_start(args, format);
    printf("\x1b[38;2;0;255;255m[%s:%i]\x1b[0m ", basename(file), line);
    vprintf(format, args);
    va_end(args);
}

#ifdef FUNCTION_CHECKER
#include "piano.h"
#include "interface.h"
extern struct Interface *interface;

void checkFunction(void *thisFunc, void *callSite){
    if(interface != NULL && interface->piano != NULL){
    // if(interface != NULL && interface->piano != NULL){
        fprintf(stderr, "variable was changed inside %p %p\n", thisFunc, callSite);
        exit(1);
    }
}

#define FUNCTION_NAME(p)do{\
    Dl_info info;\
    if(dladdr(p, &info) && info.dli_sname){\
        fprintf(stderr, "%s ", info.dli_sname);\
    }\
    else{\
        fprintf(stderr, "%p ", p);\
    }\
}while(0)

void __cyg_profile_func_enter(void *thisFunc, void *callSite){
    // checkFunction(thisFunc, callSite);
    
    fprintf(stderr, "entered function: ");
    FUNCTION_NAME(thisFunc);
    fprintf(stderr, "from ");
    FUNCTION_NAME(callSite);

    fprintf(stderr, "\n");
}

void __cyg_profile_func_exit(void *thisFunc, void *callSite){
    // checkFunction(thisFunc, callSite);
    // fprintf(stderr, "outside of function %p %p\n", thisFunc, callSite);
    fprintf(stderr, "exited function: ");
    FUNCTION_NAME(thisFunc);
    fprintf(stderr, "from ");
    FUNCTION_NAME(callSite);

    fprintf(stderr, "\n");
}
#endif
