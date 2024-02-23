#include "vector.h"

struct Vector *newVector(){
    struct Vector *vector = malloc(sizeof(struct Vector));
    
    vector->maxSize = DEFAULT_MAX_SIZE;
    vector->size = 0;
    vector->data = malloc(sizeof(void*) * DEFAULT_MAX_SIZE);

    return vector;
}

void vectorPush(struct Vector *vector, void *val){
    if(vector->maxSize <= vector->size){
        vector->maxSize *= 2;
        vector->data = realloc(vector->data, sizeof(void*) * vector->maxSize);
    }

    vector->data[vector->size++] = val;
}

bool vectorPop(struct Vector *vector, void **val){
    if(vector->size <= 0){
        return false;
    }
    
    *val = vector->data[vector->size--];
    return true;
}

void freeVector(struct Vector *vector){
    free(vector->data);
    free(vector);
}
