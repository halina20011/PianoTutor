#include "graphics.h"
// #include "midiDevice.h"
#include <stdint.h>

#include "shaders.h"

float textWidth, textHeight, screenRatio;
GLuint textureSamplerUniform;

extern float timer, speed;
extern bool paused;
bool run = true;

float pianoHeight = 0.5;

GLFWwindow *window;
GLint colorUniform, textureColorUniform;
GLuint shaderProgram, textureProgram;
GLuint vao, textureVao, vbo, textureVbo;

uint8_t wholeBitmap[BITMAP_SIZE];
GLuint textures[TEXTURE_COUNT];

// GLint compileShader(char *fileName, int type){
GLint compileShader(const char *shaderSource, int type){
    // printf("shader %s\n", shaderSource);
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char * const*)&shaderSource, NULL);
    glCompileShader(shader);
    // free(shaderSource);

    char buffer[512];
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    glGetShaderInfoLog(shader, 512, NULL, buffer);
    if(status != GL_TRUE){
        // fprintf(stderr, "failed to compile a shader: [%s]\n", );
        fprintf(stderr, "%s\n", buffer);
        exit(1);
    }

    return shader;
}

float getFloatColor(uint8_t val){
    return (float)val / (float)0xff;
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods){
    if(key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE){
        run = false;
    }

    if(action == GLFW_RELEASE){
        switch(key){
            case GLFW_KEY_SPACE:
                paused = !paused;
                break;
            case GLFW_KEY_UP:
                speed += 0.1;
                break;
            case GLFW_KEY_DOWN:
                speed -= 0.1;
                break;
            case GLFW_KEY_RIGHT:
                timer += 1.0;
                break;
            case GLFW_KEY_LEFT:
                timer -= 1.0;
                break;
        }
    }
}

void framebufferSizeCallback(GLFWwindow *w, int width, int height){
    screenRatio = (float)width / (float)height;
    printf("%i %i %f\n", width, height, screenRatio);
    glViewport(0, 0, width, height);
}

void loadBitmap(const char *fileName){
    FILE *file = fopen(fileName, "rb");
    if(!file){
        fprintf(stderr, "failed to read a bitmap\n");
        exit(1);
    }

    fread(wholeBitmap, 1, BITMAP_SIZE, file);
    fclose(file);

    // for(int y = 0; y < BITMAP_HEIGHT; y++){
    //     for(int x = 0; x < BITMAP_WIDTH; x++){
    //         wholeBitmap[y * BITMAP_WIDTH + x] = 1.0;
    //     }
    // }
    // for(int y = 0; y < BITMAP_HEIGHT; y++){
    //     for(int x = 0; x < WHOLE_BITMAP_WIDTH; x++){
    //         int index = (y * WHOLE_BITMAP_WIDTH + x);
    //         // int index = (y * BITMAP_WIDTH + x) * 3;
    //         // wholeBitmap[index] = y/1.;
    //         // wholeBitmap[index + 1] = x/1.0;
    //         // wholeBitmap[index] = (x+y)/2.0;
    //         wholeBitmap[index] = (x+y)/2.0;
    //     }
    // }
}

void loadTextures(){
    uint8_t tempTexture[TEXTURE_SIZE * TEXTURE_SIZE * BITMAP_COLOR];
    int offset = 0;
    for(int i = 0; i < TEXTURE_COUNT; i++){
        glGenTextures(1, &(textures[i]));
        printf("generated %i texture name: %i\n", i, textures[i]);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        // 
        for(int y = 0; y < TEXTURE_SIZE; y++){
            for(int x = 0; x < TEXTURE_SIZE; x++){
                if(BITMAP_COLOR == 1){
                    int fromIndex = (y * TEXTURE_SIZE + x + offset);
                    int index = ((TEXTURE_SIZE - y - 1) * TEXTURE_SIZE + x);
                    tempTexture[index] = wholeBitmap[fromIndex];
                }
                else{
                    int fromIndex = (y * TEXTURE_SIZE + x) * 3 + offset;
                    int index = (y * TEXTURE_SIZE + x) * 3;
                    tempTexture[index] = wholeBitmap[fromIndex];
                    tempTexture[index + 1] = wholeBitmap[fromIndex + 1];
                    tempTexture[index + 2] = wholeBitmap[fromIndex + 2];
                }
            }
        }
       // for(int y = 0; y < TEXTURE_SIZE; y++){
       //      for(int x = 0; x < TEXTURE_SIZE; x++){
       //          int index = (y * TEXTURE_SIZE + x) * 3;
       //          wholeBitmap[index] = y/1.0;
       //          wholeBitmap[index + 1] = x/1.0;
       //          wholeBitmap[index + 2] = (x+y)/2.0;
       //          // wholeBitmap[index] = (x+y)/2.0;
       //      }
       //  }

        offset += TEXTURE_SIZE * TEXTURE_SIZE * BITMAP_COLOR;
        
        // char buffer[100];
        // sprintf(buffer, "build/texture%i.raw", i);
        // printf("file: >%s<\n", buffer);
        // FILE *file = fopen(buffer, "w");
        // if(!file){
        //     exit(1);
        // }
        //
        // fwrite(tempTexture, 1, TEXTURE_SIZE * TEXTURE_SIZE, file);
        // fclose(file);

        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BITMAP_WIDTH, BITMAP_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, tempTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // if(BITMAP_COLOR == 1){
        //     printf("RED mode\n");
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, tempTexture);
        // }
        // else{
        //     printf("RGB mode\n");
            // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, tempTexture);
            // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BITMAP_WIDTH, BITMAP_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, wholeBitmap);
        // }
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BITMAP_WIDTH, BITMAP_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, tempTexture);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  BITMAP_WIDTH, BITMAP_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, wholeBitmap);
    }
}

void GLAPIENTRY messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam){
    const char *messageString = (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" );
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", messageString, type, severity, message );
}

void drawTexture(uint8_t textureIndex, float x, float y, float width, float height){
    GLint texture = textures[textureIndex];
    // printf("n %i => %i\n", textureIndex, texture);
    // printf("%i\n", texture);
    // GLfloat vertices[] = {
    //     // Positions       // Texture Coords
    //      0.0f,  0.0f,  0.0f, 0.0f,
    //      0.0f,  1.0f,  0.0f, 1.0f,
    //      1.0f,  1.0f,  1.0f, 1.0f,
    //      1.0f,  0.0f,  1.0f, 0.0f,
    // };
    
    height *= screenRatio;

    // glBindBuffer(GL_ARRAY_BUFFER, textureVbo);
    glUseProgram(textureProgram);
    glBindVertexArray(textureVao);
    glActiveTexture(GL_TEXTURE0 + texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(textureSamplerUniform, texture);

    GLfloat vertices[] = {
        x               , y             ,  0.0f, 0.0f,
        x               , y + height    ,  0.0f, 1.0f,
        x + width       , y             ,  1.0f, 0.0f,
        x + width       , y + height    ,  1.0f, 1.0f,
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
}

void drawNumber(uint32_t n, float y){
    // log10(2 ^ 32) ~= 9.6 => 10
    uint8_t buffer[10] = {};
    uint8_t bufferSize = 0;
    if(n == 0){
        buffer[bufferSize++] = 0;
    }

    while(n){
        uint8_t d = n % 10;
        n /= 10;
        buffer[bufferSize++] = d;
    }
    

    float x = 0.05;
    for(uint8_t i = bufferSize; i > 0; i--){
        // drawTexture(textures[buffer[i - 1]], x, y, textWidth, textHeight * screenRatio);
        drawTexture(buffer[i - 1], x, y, textWidth, textHeight);
        x += textWidth * 0.9;
    }
}

void setTextSize(float s){
    textWidth = s;
    textHeight = s;
}

void graphicsInit(){
    if(glfwInit() != GL_TRUE){
        fprintf(stderr, "failed to initialize GLFW\n");
        exit(1);
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // GLFWwindow *window = glfwCreateWindow(800, 600, "Piano Tutor", glfwGetPrimaryMonitor(), NULL);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Piano Tutor", NULL, NULL);
    if(!window){
        fprintf(stderr, "failed to open a GLFW window\n");
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        fprintf(stderr, "failed to initialize GLEW\n");
        exit(1);
    }
    
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(messageCallback, 0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glGenVertexArrays(1, &vao);
    glGenVertexArrays(1, &textureVao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo); // create vertex buffer Object (VBO)
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    const char vertexShaderSource[] = VERTEX_SHADER;
    const char fragmentShaderSource[] = FRAGMENT_SHADER;

    // compile shaders
    GLint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    const char textureVertexShaderSource[] = TEXTURE_VERTEX_SHADER;
    const char textureFragmentShaderSource[] = TEXTURE_FRAGMENT_SHADER;
    vertexShader = compileShader(textureVertexShaderSource, GL_VERTEX_SHADER);
    fragmentShader = compileShader(textureFragmentShaderSource, GL_FRAGMENT_SHADER);
    textureProgram = glCreateProgram();

    glAttachShader(textureProgram, vertexShader);
    glAttachShader(textureProgram, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(textureProgram);
    glUseProgram(textureProgram);

    float r = getFloatColor(0x0D);
    float g = getFloatColor(0x10);
    float b = getFloatColor(0x10);
    printf("color: %f %f %f\n", r, g, b);
    glClearColor(r, g, b, 1);

    glfwSetKeyCallback(window, keyCallback);
    
    loadBitmap("src/bitmap.raw");
    loadTextures();

    glBindVertexArray(textureVao);
    textureSamplerUniform = glGetUniformLocation(textureProgram, "textureSampler");
    textureColorUniform = glGetUniformLocation(textureProgram, "textureColor");
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);  // Position attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat))); // Texture attribute
    
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(vao);
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glUseProgram(shaderProgram);
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posAttrib);

    colorUniform = glGetUniformLocation(shaderProgram, "color");
}

void graphicsExit(){
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

void setColor(float r, float g, float b){
    glUniform3f(colorUniform, r, g, b);
}

void setColor1(float c){
    glUniform3f(colorUniform, c, c, c);
}

void setTextureColor(float c){
    glUseProgram(textureProgram);
    glUniform3f(textureColorUniform, c, c, c);
    glUseProgram(shaderProgram);
}

void clear(){
    glClear(GL_COLOR_BUFFER_BIT);
}

void swap(){
    glfwSwapBuffers(window);
}

bool isRunning(){
    return run;
}

void drawRectangle(float x, float y, float width, float height){
    float rectangle[] = {
        x           , y + height, // A
        x + width   , y + height, // b
        x           , y, // d
        x + width   , y  // c
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle), rectangle, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void drawLine(float x1, float y1, float x2, float y2){
    float line[] = {
        x1, y1,
        x2, y2,
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, 2);
}
