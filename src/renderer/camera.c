#include "camera.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

void camera_init(Camera* camera)
{
    camera->cameraPos = (vec3s){0.0f, 0.0f, 3.0f};

    camera->cameraTarget = (vec3s){0.0f, 0.0f, 0.0f};

    camera->cameraDirection = glms_vec3_normalize(glms_vec3_sub(camera->cameraPos,camera->cameraTarget));
    
    vec3s up = (vec3s){0.0f, 1.0f, 0.0f};
    camera->cameraRight = glms_vec3_normalize(glms_vec3_cross(up, camera->cameraDirection));

    camera->cameraUp = glms_vec3_cross(camera->cameraDirection, camera->cameraRight);
}

void camera_update(Camera* camera)
{
    const float radius = 10.0f;
    float camX = sin(glfwGetTime()) * radius;
    float camZ = cos(glfwGetTime()) * radius;

    camera->cameraPos = (vec3s){camX, 0.0f, camZ};
    camera->view = glms_lookat((vec3s){camX, 0.0, camZ}, 
  		                      (vec3s){0.0f, 0.0f, 0.0f}, 
  		                      (vec3s){0.0f, 1.0f, 0.0f});
}