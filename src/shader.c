#include "./shader.h"

GLuint compileShader(const char *shaderSource, int type){
    // debugf("%s\n", shaderSource);
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char * const*)&shaderSource, NULL);
    glCompileShader(shader);

    char buffer[512];
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE){
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        fprintf(stderr, "%s\n", buffer);
        exit(1);
    }

    return shader;
}

struct Shader *shaderInit(const char vertexShaderSource[], const char fragmentShaderSource[]){
    struct Shader *s = malloc(sizeof(struct Shader));

    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    s->program = glCreateProgram();

    glAttachShader(s->program, vertexShader);
    glAttachShader(s->program, fragmentShader);

    glLinkProgram(s->program);

    int status;
    glGetProgramiv(s->program, GL_LINK_STATUS, &status);
    if(!status){
        char buffer[512];
        glGetProgramInfoLog(s->program, 512, NULL, buffer);
        fprintf(stderr, "%s\n", buffer);
        exit(1);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGenVertexArrays(1, &s->vao);
    glBindVertexArray(s->vao);

    glGenBuffers(1, &s->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, s->vbo);

    glUseProgram(s->program);

    return s;
}

// wrapper around glGetUniformLocation to show errors
GLint getUniformLocation(struct Shader *shader, const GLchar *name){
    // debugf("shader %p\n", shader);
    GLint _uniformLocation = glGetUniformLocation(shader->program, name);
    if(_uniformLocation == -1){
        fprintf(stderr, "failed to get uniform location from \"%s\"\n", name);
        // exit(-1);
    }

    return _uniformLocation;
}

void shaderPrint(struct Shader *s){
    debugf("%i %i %i\n", s->program, s->vao, s->vbo);
}

void useShader(struct Shader *shader){
    glUseProgram(shader->program);
    glBindVertexArray(shader->vao);
    glBindBuffer(GL_ARRAY_BUFFER, shader->vbo);
}
