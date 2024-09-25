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

enum InputType{
    INPUT_TYPE_NONE,
    INPUT_TYPE_SWITCH,
    INPUT_TYPE_STR,
    INPUT_TYPE_INT,
    INPUT_TYPE_UINT8_T,
    INPUT_TYPE_FLOAT,
    INPUT_TYPE_ARRAY = (0xff + 1) 
};

struct Input{
    // uint8_t *types;
    // uint8_t dataSize;
    char *message;
    void *value;
    enum InputType dataType;
    void *options;
    char **flags;
    size_t flagsSize;
};

struct InputParser{
    int flagsOffset;
    int flagsColSize;
    struct Input **inputs;
    size_t inputsSize;
};

// #define SIZEOF_ARGS(TYPE, ...) (sizeof((TYPE[]){__VA_ARGS__})/sizeof(TYPE))

// #define FLAG_1_SIZE(FLAG1, VAR, TYPE, MESSAGE) (struct Input){MESSAGE, VAR, TYPE, 1, NULL, {FLAG1}}
// #define FLAG_2_SIZE(FLAG1, FLAG2, VAR, TYPE, MESSAGE) (struct Input){MESSAGE, VAR, TYPE, 2, NULL, {FLAG1, FLAG2}}
// #define FLAG_3_SIZE(FLAG1, FLAG2, FLAG3, VAR,  TYPE, MESSAGE) (struct Input){MESSAGE, VAR, TYPE, 3, NULL, {FLAG1, FLAG2, FLAG3}}
// #define FLAG_4_SIZE(FLAG1, FLAG2, FLAG3, FLAG4, VAR, TYPE, MESSAGE) (struct Input){MESSAGE, VAR, TYPE, 4, NULL, {FLAG1, FLAG2, FLAG3, FLAG4}}
// #define FLAG_ARRAY_1_SIZE(FLAG1, VAR, TYPE, OPTIONS, MESSAGE) (struct Input){MESSAGE, VAR, TYPE, 1, OPTIONS, {FLAG1}}

#define NEW_ARGUMENT(MESSAGE, TYPE, ...) {MESSAGE, VAR\
    TYPE,\
    SIZEOF_ARGS(char*, __VA_ARGS__), \
    {__VA_ARGS__}}

#define ARR(type, ...)   (type arr[] = ((type[]){__VA_ARGS__}))

// void initInputParser();
// void addArgument(uint8_t types, uint8_t dataSize, char message[], uint8_t flagsSize, ...);

void inputInit(void (*funcArguments)());
void *inputCreateOptions(enum InputType type, size_t count, ...);
void inputAddInput(void *value, enum InputType inputType, const char* message, void *options, size_t optionsSize, size_t size, ...);
void inputFree(void);

void printHelpMessage(struct InputParser inputParser);
void parseInput(int argc, char **argv);


#define ADD_FLAG1(FLAG1, VAR, TYPE, MESSAGE) inputAddInput(VAR, TYPE, MESSAGE, NULL, 0, 1, FLAG1);
#define ADD_FLAG2(FLAG1, FLAG2, VAR, TYPE, MESSAGE) inputAddInput(VAR, TYPE, MESSAGE, NULL, 0, 2, FLAG1, FLAG2);

// TODO: support more than string array
#define ADD_INPUT_OPTION(FLAG1, VAR, TYPE, MESSAGE, OPTIONS_SIZE, ...) do {\
    void *options = inputCreateOptions(INPUT_TYPE_STR, OPTIONS_SIZE, __VA_ARGS__);\
    inputAddInput(VAR, TYPE, MESSAGE, options, OPTIONS_SIZE, 1, FLAG1);\
} while(0)

