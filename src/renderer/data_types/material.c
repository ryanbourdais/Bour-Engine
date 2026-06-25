#include "material.h"

void material_uniforms_init(MaterialUniforms *uniforms, GLuint shader_program)
{
    uniforms->diffuse = glGetUniformLocation(shader_program, "material.diffuse");
    uniforms->specular = glGetUniformLocation(shader_program, "material.specular");
    uniforms->shininess = glGetUniformLocation(shader_program, "material.shininess");
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