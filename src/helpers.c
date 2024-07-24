#include "helpers.h"

void debugPrint(const char *file, int line, const char *format, ...){
    va_list args;
    va_start(args, format);
    printf("[%s:%i] ", file, line);
    vprintf(format, args);
    va_end(args);
}
