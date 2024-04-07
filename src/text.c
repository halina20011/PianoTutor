#include "text.h"

uint8_t textBitmap[BITMAP_SIZE];

void readBitmap(const char fileName[]){
    FILE *file = fopen(fileName, "rb");
    if(!file){
        fprintf(stderr, "unable to open file: %s\n", fileName);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    fread(textBitmap, 1, size, file);
    fclose(file);
}

struct Text *textInit(float *screenRatio){
    struct Text *text = malloc(sizeof(struct Text));
    text->screenRatio = screenRatio;

    readBitmap("src/bitmap.raw");

    text->shader = shaderInit(VERTEX_TEXT_SHADER, FRAGMENT_TEXT_SHADER);
    useShader(text->shader);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    text->colorUniform = getUniformLocation(text->shader, "textureColor");
    text->textureUniform = getUniformLocation(text->shader, "textureSampler");
    
    uint8_t tempTexture[TEXTURE_SIZE * TEXTURE_SIZE];

    int offset = 0;
    for(uint8_t i = 0; i < 127; i++){
        text->asciiMap[i].texture = 0;
        if(isprint(i)){
            // copy the texture
            // float sum = 0;
            for(int y = 0; y < TEXTURE_SIZE; y++){
                for(int x = 0; x < TEXTURE_SIZE; x++){
                    int fromIndex = (y * TEXTURE_SIZE + x + offset);
                    int index = ((TEXTURE_SIZE - y - 1) * TEXTURE_SIZE + x);
                    // sum += (float)textBitmap[fromIndex];
                    tempTexture[index] = textBitmap[fromIndex];
                }
            }
            
            offset += TEXTURE_SIZE * TEXTURE_SIZE;

            int lastPixel = offset - 1;
            // uint8_t offsetLeft = textBitmap[lastPixel - 1];
            uint8_t width = textBitmap[lastPixel - 1];
            uint8_t offsetTop = textBitmap[lastPixel];
            // printf("%c %i, %i\n", i, offsetLeft, offsetTop);

            GLuint textureName;
            glGenTextures(1, &textureName);
            glBindTexture(GL_TEXTURE_2D, textureName);
            // printf("char %c => texure name: %i avg %f\n", i, textureName, sum / (float)(w * h));
            text->asciiMap[i].texture = textureName;
            text->asciiMap[i].width = width;
            text->asciiMap[i].bearingTop = offsetTop;
            // text->asciiMap[i].bearingLeft = offsetLeft;
            // printf("t => %c %i %i\n", i, i, textureName);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                        
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, tempTexture);
        }
    }

    text->asciiMap[' '].width = text->asciiMap['A'].width;

    return text;
}

void textDraw(struct Text *text, char *str, float x, float y, float size){
    float width = size;
    float height = size * (*text->screenRatio);
    useShader(text->shader);

    glDisable(GL_DEPTH_TEST);
    for(int i = 0; str[i]; i++){
        struct CharInfo charInfo = text->asciiMap[(int)str[i]];
        GLint texture = charInfo.texture;
        uint8_t o = charInfo.bearingTop;
        float offsetY = ((float)o / (float)TEXTURE_SIZE) * size;

        float vertices[] = {
            x               , y + offsetY             ,  0.0f, 0.0f,
            x               , y + offsetY + height    ,  0.0f, 1.0f,
            x + width       , y + offsetY             ,  1.0f, 0.0f,
            x + width       , y + offsetY + height    ,  1.0f, 1.0f,
        };

        // printf("texture %i %i\n", texture, str[i]);
        glActiveTexture(GL_TEXTURE0 + texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(text->textureUniform, texture);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        x += ((float)charInfo.width / (float)TEXTURE_SIZE) * size * 1.2;
    }
}

void textColor(struct Text *text, struct TextColor color){
    glUniform3fv(text->colorUniform, 1, (float*)&color);
}
