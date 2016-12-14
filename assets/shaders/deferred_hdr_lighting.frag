#version 410 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    vec3 Position;
    uint pad0;
    vec3 Color;
    uint pad1;
    float Linear;
    float Quadratic;
    uint pad2;
    uint pad3;
};

const int NR_LIGHTS = 64;

//uniform Light lights[NR_LIGHTS];

layout (std140) uniform light_block {
    Light lights[NR_LIGHTS];
};


uniform vec3 viewPos;

uniform uint showNormals = 1;

// HDR bloom
const float bloom_thresh_min = 0.8;
const float bloom_thresh_max = 1.2;

const float PI = 3.14159265359;

float D_GGX_TR(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{


//    // Retrieve data from gbuffer
    ivec2 tex_coord = ivec2(gl_FragCoord.xy);

    vec3 FragPos = vec3(texelFetch(gPosition, ivec2(tex_coord),0));
    vec3 Normal = vec3(texelFetch(gNormal, ivec2(tex_coord), 0));
    vec4 temp = texelFetch(gAlbedoSpec, ivec2(tex_coord), 0);
    vec3 Diffuse = temp.rgb;
    float Specular = temp.a;

//    vec3 F0 = vec3(0.04);
//    F0      = mix(F0, Diffuse, metalness);

//    vec3 FragPos = texture(gPosition, TexCoords).rgb;
//    vec3 Normal = texture(gNormal, TexCoords).rgb;
//    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
//    float Specular = texture(gAlbedoSpec, TexCoords).a;

    // Then calculate lighting as usual
//    vec3 lighting  = Diffuse * 0.08; // hard-coded ambient component
    vec3 lighting  = vec3(0.0);
    vec3 viewDir  = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // Diffuse
        //        vec3 templight = vec3(0.5f * i, 0.5 * i, 0.5 * i);
        vec3 lightDir = normalize(lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;

        // Specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
//        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        float spec = D_GGX_TR(Normal, halfwayDir, 0.7);
        vec3 specular = lights[i].Color * spec * Specular;

        // Attenuation
        float distance = length(lights[i].Position - FragPos);
//        float attenuation = 50.0 / (pow(distance, 2.0) +  + 1.0);
        float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);

        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;

    }

//    FragColor = vec4(lighting, 1.0);

    FragColor = vec4(mix(lighting, Normal, showNormals),1.0);

    // ------------------------
    /// HDR

    // Calculate luminance
    float Y = dot(lighting, vec3(0.299, 0.587, 0.144));

    // Threshold color based on its luminance and write it to
    // the second output
    lighting = lighting * 4.0 * smoothstep(bloom_thresh_min, bloom_thresh_max, Y);
    BrightColor = vec4(lighting, 1.0);

//    if(showNormals == 1) {
//        FragColor = vec4(Normal, 1.0);
//        BrightColor = vec4(0.0);
//    }

}