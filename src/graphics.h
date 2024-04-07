#ifndef GRAPHICS
#define GRAPHICS

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "midiParser.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
// #define WINDOW_WIDTH 1366
// #define WINDOW_HEIGHT 768

struct Graphics{
    GLFWwindow *window;
    int width, height;
    float screenRatio;
    float deltaTime, lastFrame;
};

struct Graphics *graphicsInit();

void graphicsExit();

float getFloatColor(uint8_t val);
uint8_t notePosition(const char *noteName, int8_t octave);

void clear();
void swap(struct Graphics *g);
bool isRunning();

void drawRectangle(float x, float y, float width, float height);
void drawLine(float x1, float y1, float x2, float y2);

void drawTexture(uint8_t textureIndex, float x, float y, float width, float height);

#endif
