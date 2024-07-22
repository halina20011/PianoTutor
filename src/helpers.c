#include "helpers.h"

void dPrintF(const char *file, int line, const char *format, ...){
    va_list args;
    va_start(args, format);
    printf("[%s:%i] ", file, line);
    printf(format, args);
    va_end(args);
}
