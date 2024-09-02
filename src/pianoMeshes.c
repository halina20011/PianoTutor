#include "pianoMeshes.h"

GLuint vertexBufferStack[VERTEX_BUFFER_SIZE] = {};
GLuint vertexBufferSizes[VERTEX_BUFFER_SIZE] = {};

void vertexBufferSet(enum VertexBufferItemType item, float *data, size_t size){
    GLuint vertexSize = (GLuint)size / 3;
    GLuint vertexPrevPosition = (item == 0) ? VERTEX_BUFFER_COUNT : vertexBufferStack[item - 1];
    GLuint vertexPosition = vertexPrevPosition - vertexSize;
    glBufferSubData(GL_ARRAY_BUFFER, vertexPosition * VERTEX_BYTES_SIZE, VERTEX_BYTES_SIZE * vertexSize, data);
    vertexBufferStack[item] = vertexPosition;
    vertexBufferSizes[item] = vertexSize;
}

GLint vertexBufferGetPosition(enum VertexBufferItemType item){
    return vertexBufferStack[item];
}

GLint vertexBufferGetSize(enum VertexBufferItemType item){
    return vertexBufferSizes[item];
}
