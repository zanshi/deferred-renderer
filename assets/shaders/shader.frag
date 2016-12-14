#version 430 core
out vec4 color;

in VS_OUT
{
    vec2 TexCoords;
} vs_in;

uniform sampler2D texture_diffuse1;

void main()
{    
    color = texture(texture_diffuse1, vs_in.TexCoords);
}