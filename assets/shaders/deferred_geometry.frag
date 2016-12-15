#version 430 core
layout (location = 0) out uvec4 color0;
layout (location = 1) out vec4 color1;

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 Tangent;
    vec3 BiTangent;
} fs_in;

layout(binding = 0) uniform sampler2D texture_diffuse1;
layout(binding = 1) uniform sampler2D texture_specular1;
layout(binding = 2) uniform sampler2D texture_normal1;

void main()
{

    vec3 N = normalize(fs_in.Normal);
    vec3 T = normalize(fs_in.Tangent);
    vec3 B = normalize(fs_in.BiTangent);
//    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T,B,N);
    vec3 nm = texture(texture_normal1, fs_in.TexCoords).xyz * 2.0 - vec3(1.0);
    nm = TBN * normalize(nm);

    uvec4 outvec0 = uvec4(0);
    vec4 outvec1 = vec4(0);

    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;



    // Pack the color and normal information into the RGB32UI framebuffer texture
    outvec0.x = packHalf2x16(color.xy);
    outvec0.y = packHalf2x16(vec2(color.z, nm.x));
    outvec0.z = packHalf2x16(nm.yz);

    // Pack the world space coordinates and the specular color info into
    // the RGB32F framebuffer texture
    outvec1.xyz = fs_in.FragPos;
    outvec1.w = texture(texture_specular1, fs_in.TexCoords).r;


    color0 = outvec0;
    color1 = outvec1;

}