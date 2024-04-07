#version 330 core
layout(location = 0) in vec2 textPosition;
layout(location = 1) in vec2 fragTextureCords;
out vec2 FragTextureCords;
void main(){
    FragTextureCords = fragTextureCords;
    gl_Position = vec4(textPosition, 0.0, 1.0);
}
