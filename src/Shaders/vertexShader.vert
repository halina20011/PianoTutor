#version 330 core
in vec3 position;

uniform mat4 modelMatrix;
uniform mat4 globalMatrix;
uniform mat4 viewMatrix;

void main(){
    gl_Position = globalMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
