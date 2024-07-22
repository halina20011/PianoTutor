#version 330 core
in vec3 position;

uniform mat4 modelMatrix;
uniform mat4 globalMatrix;

void main(){
    gl_Position = globalMatrix * modelMatrix * vec4(position, 1.0);
}
