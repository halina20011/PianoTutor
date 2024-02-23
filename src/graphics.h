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

#define TEXTURE_COUNT 17
#define TEXTURE_SIZE 128

#define BITMAP_WIDTH 128
#define BITMAP_HEIGHT (BITMAP_WIDTH * TEXTURE_COUNT)
#define BITMAP_COLOR 1
#define BITMAP_SIZE (BITMAP_WIDTH * BITMAP_HEIGHT * BITMAP_COLOR)

void graphicsInit();
void graphicsExit();

void setTextSize(float s);

float getFloatColor(uint8_t val);
uint8_t notePosition(const char *noteName, int8_t octave);

void setColor(float r, float g, float b);
void setColor1(float c);
void setTextureColor(float r);

void clear();
void swap();
bool isRunning();

void drawNumber(uint32_t n, float y);
void drawRectangle(float x, float y, float width, float height);
void drawLine(float x1, float y1, float x2, float y2);

void drawTexture(uint8_t textureIndex, float x, float y, float width, float height);

#endif
