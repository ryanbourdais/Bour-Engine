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
    camera->cameraFront = (vec3s){0.0f, 0.0f, -1.0f};

    camera->cameraSpeed = 2.5f;
    camera->cameraSensitivity = 0.1f;
    camera->cameraYaw = -90.0f;
    camera->cameraPitch = 0.0f;
}

void camera_update(Camera* camera)
{
    camera->view = glms_lookat(
        camera->cameraPos,
        glms_vec3_add(camera->cameraPos, camera->cameraFront),
        camera->cameraUp
    );
}

void camera_movement(Camera* camera, vec2s movement_axis, float delta_time)
{
    float velocity = camera->cameraSpeed * delta_time;
    vec3s right = glms_vec3_normalize(glms_vec3_cross(camera->cameraFront, camera->cameraUp));
    if (movement_axis.y > 0.0f) {
        camera->cameraPos = glms_vec3_add(
            camera->cameraPos,
            glms_vec3_scale(camera->cameraFront, velocity)
        );
    }

    if (movement_axis.y < 0.0f) {
        camera->cameraPos = glms_vec3_sub(
            camera->cameraPos,
            glms_vec3_scale(camera->cameraFront, velocity)
        );
    }

    if (movement_axis.x < 0.0f) {
        camera->cameraPos = glms_vec3_sub(
            camera->cameraPos,
            glms_vec3_scale(right, velocity)
        );
    }

    if (movement_axis.x > 0.0f) {
        camera->cameraPos = glms_vec3_add(
            camera->cameraPos,
            glms_vec3_scale(right, velocity)
        );
    }
}

void handle_mouse(Camera* camera, vec2s offsets, bool constrainPitch)
{
    float xOffset = offsets.x;
    float yOffset = offsets.y;

    xOffset *= camera->cameraSensitivity;
    yOffset *= camera->cameraSensitivity;

    camera->cameraYaw += xOffset;
    camera->cameraPitch += yOffset;

    if(constrainPitch)
    {
        if(camera->cameraPitch > 89.0f)
            camera->cameraPitch = 89.0f;
        if(camera->cameraPitch < -89.0f)
            camera->cameraPitch = -89.0f;
    }

    //TODO: implement updating camera vectors
}
/* Spin camera:
const float radius = 10.0f;
    float camX = sin(glfwGetTime()) * radius;
    float camZ = cos(glfwGetTime()) * radius;

    camera->cameraPos = (vec3s){camX, 0.0f, camZ};
    camera->view = glms_lookat((vec3s){camX, 0.0, camZ}, 
  		                      (vec3s){0.0f, 0.0f, 0.0f}, 
  		                      (vec3s){0.0f, 1.0f, 0.0f});*/