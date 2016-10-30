#version 410 core
layout (location = 0) in vec3 position;
// layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;


out VS_OUT
{
    vec2 TexCoords;
} vs_out;

layout(std140) uniform TransformBlock
{
    mat4 projection;
    mat4 view;
} transform;

uniform mat4 model;

void main()
{
    gl_Position = transform.projection * transform.view * model * vec4(position, 1.0f);
    vs_out.TexCoords = texCoords;
}