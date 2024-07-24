#include "graphics.h"
#include "interface.h"
#include "piano.h"

extern struct Interface *interface;

extern GLint shaderGlobalMatUniform;
extern GLint modelShaderGlobalMatUniform;

extern GLint shaderMatUniform;
extern GLint modelShaderMatUniform;

extern GLint shaderColorUniform;
extern GLint modelShaderColorUniform;

#define UNPACK3(val) val[0], val[1], val[2]

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a < b) ? b : a)

void GLAPIENTRY messageCallback(IGNORE GLenum source, IGNORE GLenum type, IGNORE GLuint id, GLenum severity, IGNORE GLsizei length, const GLchar* message, IGNORE const void* userParam){
    // UNUSEDS(source, id, length, userParam);
    const char *messageString = (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" );
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", messageString, type, severity, message );
}

double prevMeasureMovement = 0, prevPause = 0;
void processPollEvents(){
    double currTime = glfwGetTime();
    GLFWwindow *window = interface->g->window;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    bool left = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    bool right = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;

    if(prevMeasureMovement + 0.2f < currTime){
        size_t currMeasure = interface->piano->currMeasure;
        if(left){
            if(currMeasure == 0){
                currMeasure = interface->piano->measureSize - 1;
            }
            else{
                interface->piano->currMeasure--;
            }
            prevMeasureMovement = glfwGetTime();
        }

        if(right){
            if(currMeasure == interface->piano->measureSize - 1){
                currMeasure = 0;
            }
            else{
                interface->piano->currMeasure++;
            }
            prevMeasureMovement = glfwGetTime();
        }
    }
    
    if(!left && !right){
        prevMeasureMovement = 0;   
    }

    bool spacePressed = (glfwGetKey(interface->g->window, GLFW_KEY_SPACE) == GLFW_PRESS);
    if(prevPause + 0.01f < currTime && spacePressed){
        interface->paused = !interface->paused;
    }
    prevPause = currTime;
}

void keyCallback(IGNORE GLFWwindow *w, int key, IGNORE int scancode, int action, IGNORE int mods){\
    if(action != GLFW_PRESS){
        return;
    }

    interface->key = (char)key;
}

double prevX = -1, prevY = -1;
void cursorPosCallback(IGNORE GLFWwindow *w, double x, double y){
    float xScale = 1.f / (float)interface->g->width;
    float yScale = 1.f / (float)interface->g->height;
    // debugf("%f %f\n", xScale, yScale);

    // float sizeScale = interface->scale;

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
    interface->screenRatio = (float)width / (float)height;
}

void drawLine(float x1, float y1, float z1, float x2, float y2, float z2){
    float line[] = {
        x1, y1, z1,
        x2, y2, z2,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, 2);
}

void drawLineWeight(vec3 p1, vec3 p2, vec3 pos, float rotation, GLuint arrayBuffer, GLuint elementArrayBuffer, GLuint modelUniformLocation){
    // A ------ B
    // |   |    |
    // |---+----|
    // |   |    |
    // D------- C
    float s = 0.005f;
    // float s = 0.05;
    float d = glm_vec3_distance(p1, p2);
    float points[] = {
        - s, + s, 0, 0, 0, // A
        + s, + s, 0, 0, 0, // B
        + s, - s, 0, 0, 0, // C
        - s, - s, 0, 0, 0, // D
        - s, + s, 0 - d, 0, 0, // A2
        + s, + s, 0 - d, 0, 0, // B2
        + s, - s, 0 - d, 0, 0, // C2
        - s, - s, 0 - d, 0, 0, // D2
    };

    vec3 dir = {};
    glm_vec3_sub(p2, p1, dir);
    glm_vec3_normalize(dir);

    mat4 mat;
    glm_mat4_identity(mat);

    // glm_translate(mat, p1);
    if(dir[0] == 0 && dir[2] == 0){
        glm_look(p1, dir, (vec3){1, 0, 0}, mat);
        // glm_look((vec3){0, 0, 0}, dir, (vec3){1, 0, 0}, mat);
    }
    else{
        // glm_look((vec3){0, 0, 0}, dir, (vec3){0, 1, 0}, mat);
        glm_look(p1, dir, (vec3){0, 1, 0}, mat);
    }
    
    glm_rotate_at(mat, pos, rotation, (vec3){0, 1, 0});
    glm_mat4_inv(mat, mat);
    // glm_translate(mat, pos);
    // glm_rotate_at(mat, (vec3){0, 0, 0}, rotation, (vec3){0, 1, 0});

    // glm_look(p1, dir, (vec3){0, 1, 0}, mat);
    // glm_scale(mat, (vec3){d, d, d});
    // glm_translate(mat, p1);

    glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, (float*)mat);

    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW);

    GLubyte indices[] = {
        0, 1, 2, // start 
        0, 2, 3, // 
        4, 5, 6, // end
        4, 6, 7,
        // ------
        0, 4, 
        1, 5, 
        2, 6, 
        3, 7, 
        0, 4
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    // glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_BYTE, NULL);
    // glDrawElements(GL_TRIANGLE_STRIP, 10, GL_UNSIGNED_BYTE, NULL);
    glDrawRangeElements(GL_TRIANGLES, 0, 12, 12, GL_UNSIGNED_BYTE, (void*)0);
    glDrawRangeElements(GL_TRIANGLE_STRIP, 12, 22, 10, GL_UNSIGNED_BYTE, (void*)(12 * sizeof(GLbyte)));
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
    interface->modelShader = graphicsShaderInit();

    useShader(interface->shader);
    shaderGlobalMatUniform = getUniformLocation(interface->shader, "globalMatrix");
    shaderMatUniform = getUniformLocation(interface->shader, "modelMatrix");
    shaderColorUniform = getUniformLocation(interface->shader, "color");
    SET_COLOR(shaderColorUniform, WHITE);

    useShader(interface->modelShader);
    modelShaderGlobalMatUniform = getUniformLocation(interface->shader, "globalMatrix");
    modelShaderMatUniform = getUniformLocation(interface->modelShader, "modelMatrix");
    modelShaderColorUniform = getUniformLocation(interface->shader, "color");
    SET_COLOR(modelShaderColorUniform, WHITE);

    g->window = window;
    interface->g = g;

    return g;
}

struct Shader *graphicsShaderInit(){
    struct Shader *shader = shaderInit(VERTEX_SHADER, FRAGMENT_SHADER);
    glGenVertexArrays(1, &shader->vao);
    glBindVertexArray(shader->vao);
    glGenBuffers(1, &shader->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo);
    
    useShader(shader);

    GLint posAttrib = glGetAttribLocation(shader->program, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glEnableVertexAttribArray(posAttrib);

    return shader;
}
