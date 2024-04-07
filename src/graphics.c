#include "graphics.h"
// #include "midiDevice.h"
#include <stdint.h>

#include "shader.h"
#include "shaders.h"

float textWidth, textHeight, screenRatio;
GLuint textureSamplerUniform;

extern float timer, speed;
extern bool paused;
bool run = true;

float pianoHeight = 0.5;

GLFWwindow *window;

#define UNUSED(x) (void)(x)
#define UNUSEDS(...) (void)(__VA_ARGS__)

float getFloatColor(uint8_t val){
    return (float)val / (float)0xff;
}

void keyCallback(GLFWwindow *w, int key, int scancode, int action, int mods){
    UNUSED(w);
    UNUSED(scancode);
    UNUSED(mods);
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
    struct Graphics *g = glfwGetWindowUserPointer(w);
    g->width = width;
    g->height = height;
    g->screenRatio = (float)width / (float)height;
    printf("%i %i %f\n", width, height, screenRatio);
    glViewport(0, 0, width, height);
}

void GLAPIENTRY messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam){
    UNUSED(source);
    UNUSED(id);
    UNUSED(length);
    UNUSED(userParam);
    const char *messageString = (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" );
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", messageString, type, severity, message );
}

struct Graphics *graphicsInit(){
    struct Graphics *g = malloc(sizeof(struct Graphics));
    g->deltaTime = 0;
    g->lastFrame = 0;

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
    glfwSetWindowUserPointer(window, g);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwGetFramebufferSize(window, &g->width, &g->height);
    g->screenRatio = (float)g->width / (float)g->height;

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


    float _r = getFloatColor(0x0D);
    float _g = getFloatColor(0x10);
    float _b = getFloatColor(0x10);
    // printf("color: %f %f %f\n", _r, _g, _b);
    glClearColor(_r, _g, _b, 1);

    glfwSetKeyCallback(window, keyCallback);
    
    g->window = window;
    return g;
}

void graphicsExit(){
    glfwTerminate();
}

void clear(){
    glClear(GL_COLOR_BUFFER_BIT);
}

void swap(struct Graphics *g){
    glfwSwapBuffers(g->window);
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
