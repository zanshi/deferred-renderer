#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
} vs_out;


layout(std140) uniform TransformBlock
{
    mat4 projection;
    mat4 view;
} transform;

uniform mat4 model;

void main()
{
    vec4 worldPos = model * vec4(position, 1.0f);
    vs_out.FragPos = worldPos.xyz;
    gl_Position = transform.projection * transform.view * worldPos;
    vs_out.TexCoords = texCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalMatrix * normal;
}