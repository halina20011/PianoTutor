
#define FRAGMENT_SHADER "#version 330 core\n"\
    "uniform vec3 color;\n"\
    "out vec4 outColor;\n"\
    "void main(){\n"\
    "    outColor = vec4(1, 1, 1, 1.0);\n"\
    "}\n"

#define VERTEX_SHADER "#version 330 core\n"\
    "in vec3 position;\n"\
    "\n"\
    "uniform mat4 modelMatrix;\n"\
    "\n"\
    "void main(){\n"\
    "    gl_Position = modelMatrix * vec4(position, 1.0);\n"\
    "}\n"
