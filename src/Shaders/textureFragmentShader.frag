#version 330 core
in vec2 TextureCoordinates;
uniform vec3 textureColor;
uniform sampler2D textureSampler;
out vec4 fragColor;
void main(){
    float t = texture(textureSampler, TextureCoordinates).x;
    fragColor = vec4(textureColor.x, textureColor.y, textureColor.z, t);
}
