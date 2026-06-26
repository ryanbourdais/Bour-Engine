#include "material.h"

void material_uniforms_init(MaterialUniforms *uniforms, GLuint shader_program)
{
    uniforms->diffuse = glGetUniformLocation(shader_program, "material.diffuse");
    uniforms->specular = glGetUniformLocation(shader_program, "material.specular");
    uniforms->shininess = glGetUniformLocation(shader_program, "material.shininess");
    uniforms->diffuse_color = glGetUniformLocation(shader_program, "material.diffuseColor");
    uniforms->alpha_mode = glGetUniformLocation(shader_program, "material.alphaMode");
    uniforms->alpha_cutoff = glGetUniformLocation(shader_program, "material.alphaCutoff");
}
void upload_material_samplers(MaterialUniforms *uniforms)
{
    glUniform1i(uniforms->diffuse, 0);
    glUniform1i(uniforms->specular, 1);
}
void upload_material_shininess(MaterialUniforms *uniforms, float shininess)
{
    glUniform1f(uniforms->shininess, shininess);
}

void upload_material_diffuse_color(MaterialUniforms *uniforms, vec4s diffuse_color)
{
    glUniform4fv(uniforms->diffuse_color, 1, diffuse_color.raw);
}

void upload_material_alpha(MaterialUniforms *uniforms, AlphaMode alpha_mode, float alpha_cutoff)
{
    glUniform1i(uniforms->alpha_mode, (GLint)alpha_mode);
    glUniform1f(uniforms->alpha_cutoff, alpha_cutoff);
}