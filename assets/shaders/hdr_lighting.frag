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

const int NR_LIGHTS = 16;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

// HDR bloom
uniform float bloom_thresh_min = 0.8;
uniform float bloom_thresh_max = 1.2;

void main()
{

//    // Retrieve data from gbuffer
    ivec2 tex_coord = ivec2(gl_FragCoord.xy);

    vec3 FragPos = vec3(texelFetch(gPosition, tex_coord,0));
    vec3 Normal = vec3(texelFetch(gNormal, tex_coord, 0));
    vec4 temp = texelFetch(gAlbedoSpec, tex_coord, 0);

    vec3 Diffuse = temp.rgb;
    float Specular = temp.a;

//    vec3 FragPos = texture(gPosition, tex_coord).rgb;
//    vec3 Normal = texture(gNormal, tex_coord).rgb;
//    vec3 Diffuse = texture(gAlbedoSpec, tex_coord).rgb;
//    float Specular = texture(gAlbedoSpec, tex_coord).a;

    // Then calculate lighting as usual
//    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 lighting  = vec3(0.0, 0.0, 0.0);
    vec3 viewDir  = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // Diffuse
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
//    FragColor = vec4(lighting, 1.0);
    FragColor = vec4(lighting, 1.0);

    // Calculate luminance
    float Y = dot(lighting, vec3(0.299, 0.587, 0.144));

    // Threshold color based on its luminance and write it to
    // the second output
    lighting = lighting * 4.0 * smoothstep(bloom_thresh_min, bloom_thresh_max, Y);
    BrightColor = vec4(lighting, 1.0);

}