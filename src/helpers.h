#ifndef HELPERS
#define HELPERS

#include <stdio.h>
#include <stdarg.h>

#define IGNORE __attribute__ ((unused))

#define UNUSED(x) (void)(x)
#define UNUSEDS(...) (void)(__VA_ARGS__)

#define ABS(a) ((a < 0) ? -a : a)

void debugPrint(const char *file, int line, const char *format, ...);
#define debugf(format, args...) debugPrint(__FILE__, __LINE__, format, ##args)
// #define debugf(format, args...) printf(format, ##args)

#endif
