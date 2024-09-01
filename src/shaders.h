
#define VERTEX_SHADER "#version 330 core\n"\
    "in vec3 position;\n"\
    "\n"\
    "uniform mat4 modelMatrix;\n"\
    "uniform mat4 globalMatrix;\n"\
    "uniform mat4 viewMatrix;\n"\
    "\n"\
    "void main(){\n"\
    "    gl_Position = globalMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);\n"\
    "}\n"

#define FRAGMENT_SHADER "#version 330 core\n"\
    "uniform vec4 color;\n"\
    "out vec4 outColor;\n"\
    "void main(){\n"\
    "    outColor = color;\n"\
    "}\n"
