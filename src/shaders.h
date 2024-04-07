
#define FRAGMENT_TEXT_SHADER "#version 330 core\n"\
    "in vec2 FragTextureCords;\n"\
    "uniform vec3 textureColor;\n"\
    "uniform sampler2D textureSampler;\n"\
    "out vec4 FragmentColor;\n"\
    "void main(){\n"\
    "    float t = texture(textureSampler, FragTextureCords).x;\n"\
    "    FragmentColor = vec4(textureColor, t);\n"\
    "}\n"

#define FRAGMENT_SHADER "#version 330 core\n"\
    "uniform vec3 color;\n"\
    "out vec4 outColor;\n"\
    "void main(){\n"\
    "    outColor = vec4(color, 1.0);\n"\
    "}\n"

#define VERTEX_SHADER "#version 330 core\n"\
    "in vec2 position;\n"\
    "void main(){\n"\
    "    gl_Position = vec4(position.x, position.y, 0.0, 1.0);\n"\
    "}\n"

#define VERTEX_TEXT_SHADER "#version 330 core\n"\
    "layout(location = 0) in vec2 textPosition;\n"\
    "layout(location = 1) in vec2 fragTextureCords;\n"\
    "out vec2 FragTextureCords;\n"\
    "void main(){\n"\
    "    FragTextureCords = fragTextureCords;\n"\
    "    gl_Position = vec4(textPosition, 0.0, 1.0);\n"\
    "}\n"
