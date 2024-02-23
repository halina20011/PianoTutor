#version 330 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 textureCoordinates;
out vec2 TextureCoordinates;
void main(){
    gl_Position = vec4(pos.x - 1, pos.y - 0.5, 0.0, 1.0);
    TextureCoordinates = textureCoordinates;
}
