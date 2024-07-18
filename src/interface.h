#ifndef INTERFACE
#define INTERFACE

#include "graphics.h"
#include "text.h"

struct Interface{
    int width, height;
    float screenRatio;
    char key;
    struct Graphics *g;
    struct Text *text;
    struct Shader *shader, *modelShader;
    struct Piano *piano;
};

#endif
