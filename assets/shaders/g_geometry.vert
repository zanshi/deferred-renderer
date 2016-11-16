#version 330 core
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


layout(std140) uniform TransformBlock
{
    mat4 projection;
    mat4 view;
} transform;

uniform mat4 model;

void main()
{
//    mat4 mv_matrix = transform.view * model;
//    gl_Position = transform.projection * mv_matrix * vec4(position, 1.0f);
//
//    vs_out.FragPos = (model * vec4(position,1.0f)).xyz;
//    vs_out.Normal = mat3(model) * normal;
//    vs_out.Tangent = tangent;
//    vs_out.BiTangent = bitangent;
//    vs_out.TexCoords = texCoords;


    vec4 worldPos = model * vec4(position, 1.0f);
    vs_out.FragPos = worldPos.xyz;
    gl_Position = transform.projection * transform.view * worldPos;
    vs_out.TexCoords = texCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));

    vs_out.Normal = normalMatrix * normal;
//    vs_out.Normal = mat3(model) * normal;

    vs_out.Tangent = normalMatrix * tangent;

    vs_out.BiTangent = normalMatrix * bitangent;

}