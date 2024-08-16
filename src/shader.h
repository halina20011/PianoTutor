#ifndef SHADER
#define SHADER

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

#include "helpers.h"

enum BufferObjects{
    MESH_VERTEX_BUFFER_OBJECT,
    DRAW_VERTEX_BUFFER_OBJECT,
    KEYBOARD_VERTEX_BUFFER_OBJECT
};

struct Shader{
    GLint program;
    // vertex array object
    // stores the vertex attributes
    GLuint vao;
    // vertex buffer object
    GLuint vbo;
};

GLuint compileShader(const char *shaderSource, int type);

struct Shader *shaderInit(const char vertexShaderSource[], const char fragmentShaderSource[]);
GLint getUniformLocation(struct Shader *shader, const GLchar *name);

void shaderPrint(struct Shader *s);
void useShader(struct Shader *shader);

#endif
