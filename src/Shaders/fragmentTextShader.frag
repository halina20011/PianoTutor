#version 330 core
in vec2 FragTextureCords;
uniform vec3 textureColor;
uniform sampler2D textureSampler;
out vec4 FragmentColor;
void main(){
    float t = texture(textureSampler, FragTextureCords).x;
    FragmentColor = vec4(textureColor, t);
}
