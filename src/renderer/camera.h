#pragma once
#include <glad/glad.h>
#include <cglm/struct.h>

typedef struct Camera {
    vec3s cameraPos;
    vec3s cameraTarget;
    vec3s cameraDirection;
    vec3s cameraRight;
    vec3s cameraUp;
    vec3s cameraFront;
    mat4s view;
    float cameraSpeed;
    float cameraSensitivity;
    float cameraYaw;
    float cameraPitch;
    float cameraFOV;
} Camera;

void camera_init(Camera* camera);
void camera_update(Camera* camera);
void camera_movement(Camera* camera, vec2s movement_axis, float delta_time);
void handle_mouse(Camera* camera, vec2s offsets, bool constrainPitch);
void update_camera_settings(Camera* camera, mat4 projection, GLint projection_location, float speed, float sensitivity, float fov);