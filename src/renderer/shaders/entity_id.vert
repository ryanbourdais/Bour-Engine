#version 410 core

layout(location = 0) in vec3 vp;

layout(std140) uniform CameraBlock
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(vp, 1.0);
}
