#include "text.h"
#include "interface.h"

extern struct Interface *interface;

struct Text *textInit(struct Shader *shader, float *screenRatio){
    struct Text *text = malloc(sizeof(struct Text));
    text->screenRatio = screenRatio;

    FILE *file = fopen("Assets/text.bin", "rb");
    if(!file){
        fprintf(stderr, "unable to open file: %s\n", "Assets/text.bin");
        exit(1);
    }

    int8_t numberOfChar = 0;
    fread(&numberOfChar, sizeof(uint8_t), 1, file);
    debugf("number of characters %i\n", numberOfChar);
    uint32_t bufferSize = 0;
    fread(&bufferSize, sizeof(uint32_t), 1, file);

    uint32_t bufferIndex = 0;
    float *buffer = malloc(sizeof(float) * bufferSize);

    for(int8_t i = 0; i < numberOfChar; i++){
        char c;
        fread(&c, sizeof(char), 1, file);
        uint32_t size;
        fread(&size, sizeof(uint32_t), 1, file);
        // fprintf(stderr, "%i %i + %i =>\n", bufferSize, bufferIndex, size);
        float *data = &buffer[bufferIndex];
        fread(data, sizeof(float), size, file);

        text->asciiMap[(int)c].index = bufferIndex / 5;
        text->asciiMap[(int)c].size = size / 5;
        float width = 0;
        for(uint32_t j = 0; j < size; j += 5){
            if(width < data[j]){
                width = data[j];
            }
        }
        text->asciiMap[(int)c].width = width;

        bufferIndex += size;
    }

    fclose(file);

    glGenVertexArrays(1, &text->VAO);
    glBindVertexArray(text->VAO);

    glGenBuffers(1, &text->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, text->VBO);

    GLint posAttrib = glGetAttribLocation(shader->program, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
    GLint textAttrib = glGetAttribLocation(shader->program, "texture");
    glVertexAttribPointer(textAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(posAttrib);
    glEnableVertexAttribArray(textAttrib);

    // float buffer2[] = {
    //     0, 0, 0, 0, 0,
    //     1, 0, 0, 0, 0,
    //     0, 1, 0, 0, 0,
    // };
    // glBufferData(GL_ARRAY_BUFFER, sizeof(buffer2), buffer2, GL_STATIC_DRAW);
    debugf("%p\n", buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferSize, buffer, GL_STATIC_DRAW);
    
    // for(int i = 0; i < 20; i++){
    //     debugf("%f ", buffer[i]);
    //     if((i+1) % 5 == 0){
    //         debugf("\n");
    //     }
    // }
    
    return text;
}

// float textSetHeightPx(uint16_t height){
//     float h = ((float)height / (float)interface->g->height);
//     text->height = h;
//     float s = h * 2.0f;
//     text->scale = s;
//     // debugf("%i %f %f\n", height, h, s);
//     return h * 2;
// }

float textSetHeight(float height){
    interface->text->height = height;
    interface->text->scale = height * 2.0f;
    return height * 2.0f;
}

float textGetWidth(char *str){
    size_t s = strlen(str);
    return (float)s * interface->text->scale * interface->text->asciiMap[(int)'H'].width;
}

float textDrawOnScreen(char *str, float x, float y, GLint modelUniformLocation){
    glBindVertexArray(interface->text->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, interface->text->VBO);

    // glBindBuffer(GL_ARRAY_BUFFER, text->VBO);
    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, (vec3){x, y, 0});
    float s = interface->text->scale;
    glm_scale(model, (vec3){s, s * (*(interface->text->screenRatio)), s});

    float currOffset = 0;
    int i = 0;
    for(; str[i]; i++){
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)model);
        if(str[i] == ' '){
            float offset = interface->text->asciiMap[(int)'H'].width;
            glm_translate(model, (vec3){offset, 0, 0});
            currOffset += offset * s;
            continue;
        }
        
        struct TextInfo *tInfo = &interface->text->asciiMap[(int)str[i]];

        uint32_t start = tInfo->index;
        uint32_t size = tInfo->size;
        float offset = tInfo->width;

        glDrawArrays(GL_TRIANGLES, start, size);
        glm_translate(model, (vec3){offset, 0, 0});

        currOffset += offset * s;
    }
    
    return currOffset;
}
