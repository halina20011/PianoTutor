#include "graphics.h"
#include "interface.h"
#include "piano.h"

extern struct Interface *interface;

extern GLuint elementArrayBuffer;

extern GLint globalMatUniform;
extern GLint localMatUniform;
extern GLint colorUniform;

#define UNPACK3(val) val[0], val[1], val[2]

void GLAPIENTRY messageCallback(IGNORE GLenum source, IGNORE GLenum type, IGNORE GLuint id, GLenum severity, IGNORE GLsizei length, const GLchar* message, IGNORE const void* userParam){
    // UNUSEDS(source, id, length, userParam);
    const char *messageString = (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" );
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", messageString, type, severity, message );
}

double prevMeasureMovement = 0, prevHome = 0;
void processPollEvents(){
    double currTime = glfwGetTime();
    GLFWwindow *window = interface->g->window;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    bool left = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    bool right = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;

    if(prevMeasureMovement + 0.2f < currTime){
        size_t currMeasure = interface->piano->sheet->currMeasure;
        if(left){
            if(currMeasure == 0){
                currMeasure = interface->piano->measureSize - 1;
            }
            else{
                interface->piano->sheet->currMeasure--;
            }
            prevMeasureMovement = glfwGetTime();
        }

        if(right){
            if(currMeasure == interface->piano->measureSize - 1){
                currMeasure = 0;
            }
            else{
                interface->piano->sheet->currMeasure++;
            }
            prevMeasureMovement = glfwGetTime();
        }
    }
    
    if(!left && !right){
        prevMeasureMovement = 0;   
    }
    
    bool homePressed = (glfwGetKey(interface->g->window, GLFW_KEY_HOME) == GLFW_PRESS);
    if(homePressed && prevHome + 0.01f < currTime){
        interface->xPos = 0;
        interface->yPos = 0;
        interface->scale = 1.0f;
        prevHome = currTime;
    }
}

void keyCallback(IGNORE GLFWwindow *w, int key, IGNORE int scancode, int action, IGNORE int mods){\
    if(key == 'B' && action == GLFW_RELEASE){
        interface->showBoudningBox = !interface->showBoudningBox;
    }
    
    if(key == ' ' && action == GLFW_RELEASE){
        interface->paused = !interface->paused;
    }

    if(action != GLFW_PRESS){
        return;
    }

    interface->key = (char)key;
}

double prevX = -1, prevY = -1;
void cursorPosCallback(IGNORE GLFWwindow *w, double x, double y){
    float xScale = (1.f / ((float)interface->g->width * interface->scale)) * interface->g->screenRatio;
    float s = (float)interface->g->width / (float)interface->g->height;
    float yScale = (1.f / ((float)interface->g->height) * (1.0f/interface->scale)) * s;
    // debugf("%f %f\n", xScale, yScale);

    if(prevX == -1){
        prevX = x;
        prevY = y;
    }

    if(interface->drag){
        float alphaX = x - prevX;
        float alphaY = y - prevY;
        interface->xPos += alphaX * xScale;
        interface->yPos -= alphaY * yScale;
    }

    prevX = x;
    prevY = y;
}

void mouseButtonCallback(IGNORE GLFWwindow *w, int button, int action, IGNORE int modes){
    if(button == GLFW_MOUSE_BUTTON_LEFT){
        interface->drag = (action == GLFW_PRESS);
        prevX = -1;
    }
}

void scrollCallback(IGNORE GLFWwindow *w, IGNORE double x, double y){
    // debugf("%f\n", y);
    if(y != 0){
        interface->scale += (float)y * 0.1f;
    }
}

void framebufferSizeCallback(IGNORE GLFWwindow *w, int width, int height){
    interface->g->width = width;
    interface->g->height = height;
    interface->g->screenRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
}

void drawLine(float x1, float y1, float z1, float x2, float y2, float z2){
    float line[] = {
        x1, y1, z1,
        x2, y2, z2,
    };
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line), line);
    glDrawArrays(GL_LINES, 0, 2);
}

void drawLineVec(vec3 p1, vec3 p2){
    float line[] = {
        p1[0], p1[1], p1[2],
        p2[0], p2[1], p2[2],
    };
    // glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line), line);
    glDrawArrays(GL_LINES, 0, 2);
}

void drawRectangle(float x1, float y1, float x2, float y2){
    float line[] = {
        x1, y1, 0,
        x1, y2, 0,
        x2, y1, 0,
        x2, y2, 0,
    };

    // glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line), line);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void drawLineWeight(vec3 p1, vec3 p2, vec3 pos, vec3 scale, float thicnkess, GLint modelUniformLocation){
    // A ----------------------- B
    // |                         | 
    // +p1---------------------p2+ width
    // |                         |
    // D ----------------------- C
    
    float d = glm_vec3_distance(p1, p2);
    float points[] = {
        p1[0]    , p1[1] + thicnkess / 2.0f, 0, // A
        p1[0] + d, p1[1] + thicnkess / 2.0f, 0, // B
        p1[0]    , p1[1] - thicnkess / 2.0f, 0, // D
        p1[0] + d, p1[1] - thicnkess / 2.0f, 0, // C
    };

    float x = p2[0] - p1[0];
    float y = p2[1] - p1[1];
    float rot = atan2f(y, x);

    mat4 mat;
    glm_mat4_identity(mat);

    // glm_rotate_at(mat, pos, rot, (vec3){0, 0, 1});

    glm_translate(mat, pos);
    glm_scale(mat, scale);

    glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)mat);

    // glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Piano Tutor", NULL, NULL);
    if(!window){
        fprintf(stderr, "failed to open glfw window\n");
        exit(1);
    }
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, g);

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetKeyCallback(window, keyCallback);
    // glfwSetCharCallback(window, characterCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwGetFramebufferSize(window, &g->width, &g->height);
    g->screenRatio = (float)g->width / (float)g->height;

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        fprintf(stderr, "failed to initialize GLEW\n");
        exit(1);
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(messageCallback, 0);

    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_SCISSOR_TEST);

    glClearColor(0, 0, 0, 1);

    interface->shader = graphicsShaderInit();

    globalMatUniform = getUniformLocation(interface->shader, "globalMatrix");
    localMatUniform = getUniformLocation(interface->shader, "modelMatrix");
    colorUniform = getUniformLocation(interface->shader, "color");
    SET_COLOR(colorUniform, WHITE);


    glGenBuffers(1, &elementArrayBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, elementArrayBuffer);
    useShader(interface->shader);

    g->window = window;
    interface->g = g;

    return g;
}

struct Shader *graphicsShaderInit(){
    struct Shader *shader = shaderInit(VERTEX_SHADER, FRAGMENT_SHADER);

    GLint posAttrib = glGetAttribLocation(shader->program, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glEnableVertexAttribArray(posAttrib);

    return shader;
}
