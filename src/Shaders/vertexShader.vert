#version 330 core
in vec2 position;
void main(){
    gl_Position = vec4(position.x - 1, position.y - 0.5, 0.0, 1.0);
}
