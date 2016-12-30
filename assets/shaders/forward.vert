#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 Tangent;
    vec3 BiTangent;
} vs_out;

// Transforms
layout(std140, binding = 0) uniform transform_block
{
    mat4 projView;
    mat4 model;
} transform;

void main()
{

    vec4 worldPos = transform.model * vec4(position, 1.0f);

    vs_out.FragPos = worldPos.xyz;
    gl_Position = transform.projView * worldPos;
    vs_out.TexCoords = texCoords;

    mat3 mat3Model = mat3(transform.model);
    vs_out.Normal = mat3Model * normal;
    vs_out.Tangent = tangent;
    vs_out.BiTangent = bitangent;

}
