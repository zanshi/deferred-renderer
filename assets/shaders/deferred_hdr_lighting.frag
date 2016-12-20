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
    float Radius;
    uint pad3;
};

const int NR_LIGHTS = 64;

layout (std140, binding = 1) uniform light_block {
    Light lights[NR_LIGHTS];
};

uniform vec3 viewPos;
uniform uint showNormals = 1;

// HDR bloom
const float bloom_thresh_min = 0.8;
const float bloom_thresh_max = 1.2;

struct fragment_info_t {
    vec3 color;
    vec3 normal;
    float specular_power;
    vec3 ws_coord;
    uint material_id;
};

void unpackGBuffer(ivec2 coord, out fragment_info_t fragment) {
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
    // Retrieve data from gbuffer
    fragment_info_t fragment;
    unpackGBuffer(ivec2(gl_FragCoord.xy), fragment);


    // Then calculate lighting as usual
        vec3 lighting  = fragment.color * 0.08; // hard-coded ambient component
//    vec3 lighting  = vec3(0.0);
    vec3 viewDir  = normalize(viewPos - fragment.ws_coord);
    for(int i = 0; i < NR_LIGHTS; ++i) {
        // Diffuse
        vec3 L = lights[i].Position - fragment.ws_coord;
        float distance = length(L);

        if(distance < lights[i].Radius) {

            vec3 lightDir = normalize(L);
            vec3 diffuse = max(dot(fragment.normal, lightDir), 0.0) * fragment.color * lights[i].Color;

            // Specular
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(fragment.normal, halfwayDir), 0.0), 16.0);
            vec3 specular = lights[i].Color * spec * fragment.specular_power;

            // Attenuation
            float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);

            lighting += (diffuse + specular) * attenuation;
        }
    }

//    FragColor = vec4(lighting, 1.0);

    FragColor = vec4(mix(lighting, fragment.normal, showNormals),1.0);

    // ------------------------
    /// HDR and bloom

    // Calculate luminance
    float Y = dot(lighting, vec3(0.299, 0.587, 0.144));

    // Threshold color based on its luminance and write it to
    // the second output
    lighting = lighting * 4.0 * smoothstep(bloom_thresh_min, bloom_thresh_max, Y);
    BrightColor = vec4(lighting, 1.0);

}
