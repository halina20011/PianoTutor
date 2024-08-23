#ifndef HELPERS
#define HELPERS

#include <stdio.h>
#include <stdarg.h>
#include <libgen.h>

#define IGNORE __attribute__ ((unused))

#define UNUSED(x) (void)(x)
#define UNUSEDS(...) (void)(__VA_ARGS__)

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a < b) ? b : a)
#define ABS(a) ((a < 0) ? -a : a)

void debugPrint(const char *file, int line, const char *format, ...);
#define debugf(format, args...) debugPrint(__FILE__, __LINE__, format, ##args)
// #define debugf(format, args...) printf(format, ##args)

void checkFunction(void *thisFunc, void *callSite) __attribute__((no_instrument_function));
void __cyg_profile_func_enter(void *thisFunc, void *callSite) __attribute__((no_instrument_function));
void __cyg_profile_func_exit(void *thisFunc, void *callSite) __attribute__((no_instrument_function));

#endif
