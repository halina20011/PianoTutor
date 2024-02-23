#ifndef VECTOR
#define VECTOR

#include <stdlib.h>
#include <stdbool.h>

#define DEFAULT_MAX_SIZE 20

struct Vector{
    void **data;
    size_t size, maxSize;
};

struct Vector *newVector();
void vectorPush(struct Vector *vector, void* val);

bool vectorPop(struct Vector *vector, void **val);
void freeVector(struct Vector *vector);

#endif
