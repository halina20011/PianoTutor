#ifndef INTERFACE
#define INTERFACE

#include "helpers.h"

#include "graphics.h"
#include "text.h"

struct Interface{
    char key;
    bool paused;

    bool drag;
    float xPos, yPos, scale;

    struct Graphics *g;
    struct Text *text;
    // struct Shader *shader, *modelShader;
    struct Shader *shader;
    struct Piano *piano;

    bool showBoudningBox;
};

#endif
