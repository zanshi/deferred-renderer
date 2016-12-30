#version 430 core
layout (location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D bright_image;

// Weights for the Gaussian blur
const float weights[] = float[](0.0024499299678342,
                                0.0043538453346397,
                                0.0073599963704157,
                                0.0118349786570722,
                                0.0181026699707781,
                                0.0263392293891488,
                                0.0364543006660986,
                                0.0479932050577658,
                                0.0601029809166942,
                                0.0715974486241365,
                                0.0811305381519717,
                                0.0874493212267511,
                                0.0896631113333857,
                                0.0874493212267511,
                                0.0811305381519717,
                                0.0715974486241365,
                                0.0601029809166942,
                                0.0479932050577658,
                                0.0364543006660986,
                                0.0263392293891488,
                                0.0181026699707781,
                                0.0118349786570722,
                                0.0073599963704157,
                                0.0043538453346397,
                                0.0024499299678342);

void main(void)
{
    vec4 c = vec4(0.0);
    // Get the transposed fragment position
    ivec2 P = ivec2(gl_FragCoord.yx) - ivec2(0, weights.length() >> 1);
    int i;

    for (i = 0; i < weights.length(); i++)
    {
        c += texelFetch(bright_image, P + ivec2(0, i), 0) * weights[i];
    }

    color = c;
}
