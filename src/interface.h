#ifndef INTERFACE
#define INTERFACE

#include "graphics.h"
#include "text.h"

struct Interface{
    float screenRatio;
    char key;
    bool paused;

    bool drag;
    float xPos, yPos, scale;

    struct Graphics *g;
    struct Text *text;
    struct Shader *shader, *modelShader;
    struct Piano *piano;
};

#endif
