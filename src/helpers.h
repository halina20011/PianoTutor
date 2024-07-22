#ifndef HELPERS
#define HELPERS

#include <stdio.h>
#include <stdarg.h>

#define IGNORE __attribute__ ((unused))

void dPrintF(const char *file, int line, const char *format, ...);
#define debugf(format, ...) debugF(__FILE__, __LINE__, format, ##__VA_ARGS__)

#endif
