#include "window.h"

struct{
    float x, y;
    float width, height;
} Window;

void setWindow(float x, float y, float width, float height){
    Window.x = x;
    Window.y = y;

    Window.width = width;
    Window.height = height;
}

float windowX(float x){
    return Window.x + (x / 1.0f) * Window.width;
}

float windowY(float y){
    return Window.y + (y / 1.0f) * Window.height;
}

float windowWidth(float width){
    return (width / 1.0f) * Window.width;
}

float windowHeight(float height){
    return (height / 1.0f) * Window.height;
}
