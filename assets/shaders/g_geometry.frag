#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT
{
    vec2 TexCoords;
    vec3 FragPos;
    vec3 Normal;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
    // Store the fragment position vector in the first gbuffer texture
    gPosition = fs_in.FragPos;
    // Also store the per-fragment normals into the gbuffer
    gNormal = normalize(fs_in.Normal);
    // And the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // Store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(texture_specular1, fs_in.TexCoords).r;
}