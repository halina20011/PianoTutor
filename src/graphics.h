#ifndef __GRAPHICS__
#define __GRAPHICS__

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define CGLM_DEFINE_PRINTS 1
#define DEBUG 1

#include <cglm/cglm.h>
#include <cglm/types.h>
#include <cglm/io.h>

#include <cglm/mat4.h>
#include <cglm/vec4.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "./shader.h"
#include "./shaders.h"

#include "helpers.h"
#include "color.h"

#define WINDOW_WIDTH 1362
#define WINDOW_HEIGHT 716

#define MAX_COMMAND_BUFFER_SIZE 200

struct Graphics{
    GLFWwindow *window;
    int width, height;
    float screenRatio;
    float deltaTime, lastFrame;
};

struct Graphics *graphicsInit();
void processPollEvents();

// events
void keyCallback(IGNORE GLFWwindow *w, int key, IGNORE int scancode, int action, IGNORE int mods);
void cursorPosCallback(IGNORE GLFWwindow *w, double x, double y);
void mouseButtonCallback(IGNORE GLFWwindow *w, int button, int action, IGNORE int modes);
void scrollCallback(IGNORE GLFWwindow *w, IGNORE double x, double y);
void framebufferSizeCallback(IGNORE GLFWwindow *w, int width, int height);

void drawLine(float x1, float y1, float z1, float x2, float y2, float z2);
void drawLineVec(vec3 p1, vec3 p2);
void drawLineWeight(vec3 p1, vec3 p2, vec3 pos, vec3 scale, float thicnkess, GLint modelUniformLocation);
void drawRectangle(float x1, float y1, float x2, float y2);

struct Shader *graphicsShaderInit();

#endif
