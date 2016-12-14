#version 410 core

out vec4 color;

uniform sampler2D brightImage;

void main()
{
    color = vec4(texture(brightImage, gl_FragCoord.yx).rgb, 1.0);

}