#ifndef HELPERS
#define HELPERS

// #define FUNCTION_CHECKER

#ifdef FUNCTION_CHECKER
#define _GNU_SOURCE
#include <link.h>
#include <dlfcn.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <libgen.h>

#define IGNORE __attribute__ ((unused))

#define UNUSED(x) (void)(x)
#define UNUSEDS(...) (void)(__VA_ARGS__)

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a < b) ? b : a)
#define ABS(a) ((a < 0) ? -a : a)

#define SET_BIT(variable, bit) (variable |= (1 << bit))
#define GET_BIT(variable, bit) (variable & (1 << bit))

void debugPrint(const char *file, int line, const char *format, ...);
bool debugPrintGetStatus(void);
void debugPrintStatus(bool status);
#define debugf(format, args...) debugPrint(__FILE__, __LINE__, format, ##args)
// #define debugf(format, args...) printf(format, ##args)
#define DEBUG_CHECK() if(!debugPrintGetStatus()) return;

void debugMeasureAll(bool b);
void debugMeasure(size_t measure);
void debugMeasures(size_t from, size_t to);
void debugMeasurePosition(size_t measure);

void checkFunction(void *thisFunc, void *callSite) __attribute__((no_instrument_function));
void __cyg_profile_func_enter(void *thisFunc, void *callSite) __attribute__((no_instrument_function));
void __cyg_profile_func_exit(void *thisFunc, void *callSite) __attribute__((no_instrument_function));

#endif
