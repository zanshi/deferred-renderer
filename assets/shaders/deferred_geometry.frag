#version 410 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 Tangent;
    vec3 BiTangent;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

void main()
{

    vec3 N = normalize(fs_in.Normal);
    vec3 T = normalize(fs_in.Tangent);
    vec3 B = normalize(fs_in.BiTangent);
//    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T,B,N);
    vec3 nm = texture(texture_normal1, fs_in.TexCoords).xyz * 2.0 - vec3(1.0);
    nm = TBN * normalize(nm);

    // Store the fragment position vector in the first gbuffer texture
    gPosition = fs_in.FragPos;
    // Also store the per-fragment normals into the gbuffer
    gNormal = nm;
    // And the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // Store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(texture_specular1, fs_in.TexCoords).r;
}