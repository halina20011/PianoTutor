#ifndef VECTOR
#define VECTOR

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "helpers.h"

#define VECTOR_TYPE_FUNCTIONS(type, name, format)\
    struct name *name ## Init(){\
        struct name *v = malloc(sizeof(struct name));\
        v->size = 0;\
        v->maxSize = 20;\
        v->data = malloc(sizeof(type) * v->maxSize);\
        return v;\
    }\
    void name ## Print(struct name *v){\
        for(size_t i = 0; i < v->size; i++){\
            printf(format ", ", v->data[i]);\
        }\
        printf("\n");\
    }\
    \
    void name ## Push(struct name *v, type val){\
        if(v->maxSize <= v->size){\
            v->maxSize *= 2;\
            v->data = realloc(v->data, sizeof( type ) * v->maxSize);\
        }\
        v->data[v->size++] = val;\
    }\
    \
    bool name ## Pop(struct name *v, type *val){\
        if(v->size <= 0){\
            return false;\
        }\
        \
        if(val == NULL){\
            return true;\
        }\
        *val = v->data[--v->size];\
        return true;\
    }\
    \
    type *name ## Duplicate(struct name *v, size_t *size){\
        size_t bSize = sizeof(type) * v->size;\
        type *array = malloc(bSize);\
        memcpy(array, v->data, bSize);\
        if(size){\
            *size = v->size;\
        }\
        return array;\
    }\
    \
    void name ## Free(struct name *v){\
        free(v->data);\
        free(v);\
    }\

#define NEW_VECTOR_TYPE(type, name)\
    struct name {\
        type *data;\
        size_t size, maxSize;\
    };\
    \
    struct name *name ## Init();\
    void name ## Print(struct name *v);\
    void name ## Push(struct name *v, type val);\
    bool name ## Pop(struct name *v, type *val);\
    type *name ## Duplicate(struct name *v, size_t *size);\
    void name ## Free(struct name *v);

#endif
