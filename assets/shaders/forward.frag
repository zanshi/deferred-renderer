#version 410 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;


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

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 Tangent;
    vec3 BiTangent;
} fs_in;


void main()
{

    vec3 N = normalize(fs_in.Normal);
    vec3 T = normalize(fs_in.Tangent);
    vec3 B = normalize(fs_in.BiTangent);
//    vec3 B = normalize(cross(N, T));

    mat3 TBN = mat3(T,B,N);
    vec3 nm = texture(texture_normal1, fs_in.TexCoords).xyz * 2.0 - vec3(1.0);
    nm = TBN * normalize(nm);

    vec3 Diffuse = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // Store specular intensity in gAlbedoSpec's alpha component
    float Specular = texture(texture_specular1, fs_in.TexCoords).r;
    vec3 Normal = normalize(nm);


        // Then calculate lighting as usual
        vec3 lighting  = Diffuse * 0.08; // hard-coded ambient component
    //    vec3 lighting  = vec3(0.0);
        vec3 viewDir  = normalize(viewPos - fs_in.FragPos);
        for(int i = 0; i < NR_LIGHTS; ++i)
        {
            // Diffuse
            //        vec3 templight = vec3(0.5f * i, 0.5 * i, 0.5 * i);
            vec3 lightDir = normalize(lights[i].Position - fs_in.FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;

            // Specular
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = lights[i].Color * spec * Specular;

            // Attenuation
            float distance = length(lights[i].Position - fs_in.FragPos);
    //        float attenuation = 50.0 / (pow(distance, 2.0) +  + 1.0);
            float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);

            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;

        }

        FragColor = vec4(lighting, 1.0);

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
        }







}