
#define FRAGMENT_SHADER "#version 330 core\n"\
    "uniform vec4 color;\n"\
    "out vec4 outColor;\n"\
    "void main(){\n"\
    "    outColor = color;\n"\
    "}\n"

#define VERTEX_SHADER "#version 330 core\n"\
    "in vec3 position;\n"\
    "\n"\
    "uniform mat4 modelMatrix;\n"\
    "uniform mat4 globalMatrix;\n"\
    "\n"\
    "void main(){\n"\
    "    gl_Position = globalMatrix * modelMatrix * vec4(position, 1.0);\n"\
    "}\n"
