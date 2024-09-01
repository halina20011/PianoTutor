#ifndef __VIEW__
#define __VIEW__

#include <stdlib.h>

#define GLEW_STATIC
#include <GL/glew.h>

#define CGLM_DEFINE_PRINTS 1
#define DEBUG 1

#include <cglm/cglm.h>
#include <cglm/types.h>
#include <cglm/io.h>

#include <cglm/mat4.h>
#include <cglm/vec4.h>

#include "helpers.h"

enum ViewItemType{
    VIEW_ITEM_TYPE_SHEET,
    VIEW_ITEM_TYPE_NOTES,
    VIEW_ITEM_TYPE_KEYBOARD
};

struct ViewItem{
    float height;
    float yStart;
};

struct View{
    size_t size;
    struct ViewItem *items;
};

void viewInit(struct View *view, size_t size);
void viewRecalc(struct View *view);
void viewReset(void);

void viewSet(struct View *view, enum ViewItemType type, float height);
void viewUse(struct View *view, enum ViewItemType type);

#endif
