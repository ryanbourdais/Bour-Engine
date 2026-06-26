#pragma once
#include <cglm/struct.h>
#include <glad/glad.h>

typedef enum AlphaMode
{
    ALPHA_MODE_OPAQUE,
    ALPHA_MODE_MASK,
    ALPHA_MODE_BLEND
} AlphaMode;

typedef struct Material {
    GLuint diffuse_texture;
    GLuint specular_texture;

    vec4s diffuse_color;

    float shininess;

    AlphaMode alpha_mode;
    float alpha_cutoff;
} Material;

typedef struct MaterialUniforms
{
    GLint diffuse;
    GLint specular;
    GLint shininess;
    GLint diffuse_color;
    GLint alpha_mode;
    GLint alpha_cutoff;
} MaterialUniforms;

void material_uniforms_init(MaterialUniforms *uniforms, GLuint shader_program);
void upload_material_samplers(MaterialUniforms *uniforms);
void upload_material_shininess(MaterialUniforms *uniforms, float shininess);
void upload_material_diffuse_color(MaterialUniforms *uniforms, vec4s diffuse_color);
void upload_material_alpha(MaterialUniforms *uniforms, AlphaMode alpha_mode, float alpha_cutoff);