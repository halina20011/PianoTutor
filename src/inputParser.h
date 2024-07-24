#define INPUT_STR
#define INPUT_FLOAT

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "helpers.h"

#define MAX_FLAG_COUNT   5
#define MAX_FLAG_LENGTH  20
#define JOIN_FLAGS_BUFFER (MAX_FLAG_COUNT * (MAX_FLAG_LENGTH + 2))

struct Input{
    // uint8_t *types;
    // uint8_t dataSize;
    char *message;
    void *variable;
    uint8_t dataType;
    uint8_t flagsSize;
    char *flags[20];
};

struct InputParser{
    int flagsOffset;
    int flagsColSize;
    size_t inputsSize;
    struct Input inputs[20];
};

enum{
    INPUT_TYPE_NONE,
    INPUT_TYPE_SWITCH,
    INPUT_TYPE_STR,
    INPUT_TYPE_INT,
    INPUT_TYPE_UINT8_T,
    INPUT_TYPE_FLOAT,
};

#define SIZEOF_ARGS(TYPE, ...) (sizeof((TYPE[]){__VA_ARGS__})/sizeof(TYPE))

#define FLAG_1_SIZE(FLAG1, VAR, TYPE, MESSAGE) (struct Input){MESSAGE, VAR, TYPE, 1, {FLAG1}}
#define FLAG_2_SIZE(FLAG1, FLAG2, VAR, TYPE, MESSAGE) (struct Input){MESSAGE, VAR, TYPE, 2, {FLAG1, FLAG2}}
#define FLAG_3_SIZE(FLAG1, FLAG2, FLAG3, VAR,  TYPE, MESSAGE) (struct Input){MESSAGE, VAR, TYPE, 3, {FLAG1, FLAG2, FLAG3}}
#define FLAG_4_SIZE(FLAG1, FLAG2, FLAG3, FLAG4, VAR, TYPE, MESSAGE) (struct Input){MESSAGE, VAR, TYPE, 4, {FLAG1, FLAG2, FLAG3, FLAG4}}

#define NEW_ARGUMENT(MESSAGE, TYPE, ...) {MESSAGE, VAR\
    TYPE,\
    SIZEOF_ARGS(char*, __VA_ARGS__), \
    {__VA_ARGS__}}

#define ARR(type, ...)   (type arr[] = ((type[]){__VA_ARGS__}))

// void initInputParser();
// void addArgument(uint8_t types, uint8_t dataSize, char message[], uint8_t flagsSize, ...);

void printHelpMessage(struct InputParser inputParser);
void parseInput(struct InputParser inputParser, int argc, char **argv);
