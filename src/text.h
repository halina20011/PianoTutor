#ifndef TEXT
#define TEXT

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define CGLM_DEFINE_PRINTS 1
#define DEBUG 1

#include <cglm/cglm.h>
#include <cglm/types.h>
#include <cglm/io.h>

#include <cglm/vec3.h>
#include <cglm/mat4.h>

#include "./shader.h"
#include "./shaders.h"

#define ASCII_TABLE_SIZE ((1 << 8) - 1)

struct TextInfo{
    uint32_t index, size;
    float width;
};

struct Text{
    GLuint textureUniform, colorUniform;
    struct TextInfo asciiMap[ASCII_TABLE_SIZE];
    GLuint VAO, VBO;
    float *screenRatio;
    float height, scale;
};

struct Text *textInit(struct Shader *shader, float *screenRatio);

float textSetHeightPx(uint16_t height);
float textSetHeight(float height);

float textGetWidth(char *str);

float textDrawOnScreen(char *str, float x, float y, GLint modelUniformLocation);
void textDraw(char *str, float x, float y, float size);

#endif
