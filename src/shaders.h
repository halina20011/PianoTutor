
#define TEXTURE_FRAGMENT_SHADER "#version 330 core\n"\
    "in vec2 TextureCoordinates;\n"\
    "uniform vec3 textureColor;\n"\
    "uniform sampler2D textureSampler;\n"\
    "out vec4 fragColor;\n"\
    "void main(){\n"\
    "    float t = texture(textureSampler, TextureCoordinates).x;\n"\
    "    fragColor = vec4(textureColor.x, textureColor.y, textureColor.z, t);\n"\
    "}\n"

#define FRAGMENT_SHADER "#version 330 core\n"\
    "uniform vec3 color;\n"\
    "out vec4 outColor;\n"\
    "void main(){\n"\
    "    outColor = vec4(color, 1.0);\n"\
    "}\n"

#define TEXTURE_VERTEX_SHADER "#version 330 core\n"\
    "layout(location = 0) in vec2 pos;\n"\
    "layout(location = 1) in vec2 textureCoordinates;\n"\
    "out vec2 TextureCoordinates;\n"\
    "void main(){\n"\
    "    gl_Position = vec4(pos.x - 1, pos.y - 0.5, 0.0, 1.0);\n"\
    "    TextureCoordinates = textureCoordinates;\n"\
    "}\n"

#define VERTEX_SHADER "#version 330 core\n"\
    "in vec2 position;\n"\
    "void main(){\n"\
    "    gl_Position = vec4(position.x - 1, position.y - 0.5, 0.0, 1.0);\n"\
    "}\n"
