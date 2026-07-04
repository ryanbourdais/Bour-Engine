#version 410 core

layout(location = 0) in vec3 vp;
layout(location = 3) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

layout(location = 4) in mat4 instanceModel;

layout(std140) uniform CameraBlock
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

uniform mat4 model;

uniform int useInstancing;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    mat4 objectModel = useInstancing == 1 ? instanceModel * model : model;

    FragPos = vec3(objectModel * vec4(vp, 1.0));
    Normal = mat3(transpose(inverse(objectModel))) * aNormal;

    gl_Position = projection * view * objectModel * vec4(vp, 1.0);
    TexCoords = aTexCoords;
}
