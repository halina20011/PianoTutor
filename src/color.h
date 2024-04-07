#ifndef COLOR
#define COLOR

#define GLEW_STATIC
#include <GL/glew.h>

#define N(val) ((float)val/ 255.0f)

struct Color{
    float r, g, b;
};

#define SET_COLOR(colorUniform, color) glUniform3fv(colorUniform, 1, (float*)&color)

#define BLACK               (struct Color){0, 0, 0}
#define WHITE               (struct Color){1, 1, 1}
#define RED                 (struct Color){1, 0, 0}
#define GREEN               (struct Color){0, 1, 0}
#define BLUE                (struct Color){0, 0, 1}
#define PURPLE              (struct Color){N(102), 0, N(204)}
#define LIGHT_PURPLE        (struct Color){N(204), 0, 1}

#endif
