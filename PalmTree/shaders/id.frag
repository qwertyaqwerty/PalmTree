#version 330 core

in vec2 TexCoords;

out vec4 color;

uniform sampler2D texture_diffuse1;

uniform int meshId;

void main()
{    
    color = vec4(meshId / 3.0, 1.0 - meshId / 3.0, 0.0, 0.0);
}
