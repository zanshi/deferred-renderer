#version 430 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

layout(binding = 0) uniform usampler2D gbuffer_tex0;
layout(binding = 1) uniform sampler2D gbuffer_tex1;

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

const int NR_LIGHTS = 32;

//uniform Light lights[NR_LIGHTS];

layout (std140) uniform light_block {
    Light lights[NR_LIGHTS];
};


uniform vec3 viewPos;

uniform uint showNormals = 1;

// HDR bloom
const float bloom_thresh_min = 0.8;
const float bloom_thresh_max = 1.2;

struct fragment_info_t
{
    vec3 color;
    vec3 normal;
    float specular_power;
    vec3 ws_coord;
    uint material_id;
};

void unpackGBuffer(ivec2 coord,
                   out fragment_info_t fragment)
{
    uvec4 data0 = texelFetch(gbuffer_tex0, ivec2(coord), 0);
    vec4 data1 = texelFetch(gbuffer_tex1, ivec2(coord), 0);
    vec2 temp;

    temp = unpackHalf2x16(data0.y);
    fragment.color = vec3(unpackHalf2x16(data0.x), temp.x);
    fragment.normal = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));
    fragment.material_id = data0.w;

    fragment.ws_coord = data1.xyz;
    fragment.specular_power = data1.w;
}

void main()
{


//    // Retrieve data from gbuffer

    fragment_info_t fragment;


    unpackGBuffer(ivec2(gl_FragCoord.xy), fragment);




//    vec3 F0 = vec3(0.04);
//    F0      = mix(F0, Diffuse, metalness);

//    vec3 FragPos = texture(gPosition, TexCoords).rgb;
//    vec3 Normal = texture(gNormal, TexCoords).rgb;
//    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
//    float Specular = texture(gAlbedoSpec, TexCoords).a;

    // Then calculate lighting as usual
//    vec3 lighting  = Diffuse * 0.08; // hard-coded ambient component
    vec3 lighting  = vec3(0.0);
    vec3 viewDir  = normalize(viewPos - fragment.ws_coord);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // Diffuse
        //        vec3 templight = vec3(0.5f * i, 0.5 * i, 0.5 * i);
        vec3 lightDir = normalize(lights[i].Position - fragment.ws_coord);
        vec3 diffuse = max(dot(fragment.normal, lightDir), 0.0) * fragment.color * lights[i].Color;

        // Specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(fragment.normal, halfwayDir), 0.0), 16.0);
//        float spec = D_GGX_TR(fragment.normal, halfwayDir, 0.7);
        vec3 specular = lights[i].Color * spec * fragment.specular_power;

        // Attenuation
        float distance = length(lights[i].Position - fragment.ws_coord);
//        float attenuation = 50.0 / (pow(distance, 2.0) +  + 1.0);
        float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);

        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;

    }

//    FragColor = vec4(lighting, 1.0);

    FragColor = vec4(mix(lighting, fragment.normal, showNormals),1.0);

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