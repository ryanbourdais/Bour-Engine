#pragma once
#include <cglm/struct.h>
#include <glad/glad.h>

typedef struct MaterialUniforms
{
    GLint diffuse;
    GLint specular;
    GLint shininess;
} MaterialUniforms;

void material_uniforms_init(MaterialUniforms *uniforms, GLuint shader_program);
void upload_material_samplers(MaterialUniforms *uniforms);
void upload_material_shininess(MaterialUniforms *uniforms, float shininess);