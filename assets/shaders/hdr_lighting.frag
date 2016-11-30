#version 410 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

//in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
    float Radius;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

uniform uint showNormals = 1;

// HDR bloom
const float bloom_thresh_min = 0.8;
const float bloom_thresh_max = 1.2;

void main()
{


//    // Retrieve data from gbuffer
    ivec2 tex_coord = ivec2(gl_FragCoord.xy);

    vec3 FragPos = vec3(texelFetch(gPosition, ivec2(tex_coord),0));
    vec3 Normal = vec3(texelFetch(gNormal, ivec2(tex_coord), 0));
    vec4 temp = texelFetch(gAlbedoSpec, ivec2(tex_coord), 0);

    vec3 Diffuse = temp.rgb;
    float Specular = temp.a;

    // Then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
//    vec3 lighting  = vec3(0.0);
    vec3 viewDir  = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // Diffuse
//        vec3 templight = vec3(0.5f * i, 0.5 * i, 0.5 * i);
        vec3 lightDir = normalize(lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;

        // Specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = lights[i].Color * spec * Specular;

        // Attenuation
        float distance = length(lights[i].Position - FragPos);
//        float attenuation = 50.0 / (pow(distance, 2.0) +  + 1.0);
        float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);

        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }

    // ------------------------
    /// HDR

    // Calculate luminance
    float Y = dot(lighting, vec3(0.299, 0.587, 0.144));

    // Threshold color based on its luminance and write it to
    // the second output
    lighting = lighting * 4.0 * smoothstep(bloom_thresh_min, bloom_thresh_max, Y);
    BrightColor = vec4(lighting, 1.0);

    if(showNormals == 1) {
        FragColor = vec4(Normal, 1.0);
        BrightColor = vec4(0.0);
    } else {
        FragColor = vec4(lighting, 1.0);
    }

}