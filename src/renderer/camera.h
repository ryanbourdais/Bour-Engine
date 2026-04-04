#pragma once
#include <glad/glad.h>
#include <cglm/struct.h>

typedef struct Camera {
    vec3s cameraPos;
    vec3s cameraTarget;
    vec3s cameraDirection;
    vec3s cameraRight;
    vec3s cameraUp;
    mat4s view;
    float cameraSpeed;
} Camera;

void camera_init(Camera* camera);
void camera_update(Camera* camera);