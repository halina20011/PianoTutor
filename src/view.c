#include "view.h"

extern GLint viewMatUniform;

#include "graphics.h"
extern GLint colorUniform;
#include "color.h"
struct Color colors[] = {
    RED, GREEN, BLUE, PURPLE
}; 

void viewInit(struct View *view, size_t size){
    view->size = size;
    view->items = malloc(sizeof(struct ViewItem) * size);
}

void viewSet(struct View *view, enum ViewItemType type, float height){
    // struct ViewItem *prevItem = (type == 0) ? NULL : &view->items[type - 1];
    // float yStart = (prevItem) ? prevItem->height + prevItem->yStart : 0;
    view->items[type].height = height;
    view->items[type].yStart = 0;
}

void viewRecalc(struct View *view){
    view->items[0].yStart = 0;
    for(size_t i = 1; i < view->size; i++){
        view->items[i].yStart = (view->items[i - 1].yStart - view->items[i - 1].height);
    }
}

void viewReset(){
    mat4 viewMatrix = {};
    glm_mat4_identity(viewMatrix);
    glUniformMatrix4fv(viewMatUniform, 1, GL_FALSE, (float*)viewMatrix);
}

void viewUse(struct View *view, enum ViewItemType type){
    // -1 <===> 1
    //  to x1 <=> x2
    // x2 - x1

    float widht = 2.0f;
    float height = view->items[type].height;
    // debugf();
    float scaleY = fabsf(height) / 2.0f;

    // float scale = view->items[type].height / 2.0f;
    float yStart = view->items[type].yStart;

    // debugf("scale: %f\n", scale);
    mat4 viewMatrix = {};
    glm_mat4_identity(viewMatrix);
    glm_translate(viewMatrix, (vec3){-1, 1 + yStart - height / 2.0f, 0});
    // glm_scale(viewMatrix, (vec3){scale, scale, scale});
    glm_scale(viewMatrix, (vec3){scaleY, scaleY, 1});
    glm_translate(viewMatrix, (vec3){widht / 2.0f, 0, 0});
    // glm_translate(viewMatrix, (vec3){1, -yStart, 0});
    glUniformMatrix4fv(viewMatUniform, 1, GL_FALSE, (float*)viewMatrix);

    // SET_COLOR(colorUniform, colors[type]);
    // drawRectangle(-1, -1, 1, 1);
    // SET_COLOR(colorUniform, colors[type + 1]);
    // drawLine(-1, 0, 0, 1, 0, 0);
}
