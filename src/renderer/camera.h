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

#define CAMERA_UBO_BINDING 0

typedef struct CameraUniformData
{
    mat4 view;
    mat4 projection;
    vec4 view_pos;
} CameraUniformData;

void camera_init(Camera* camera);
void upload_camera(GLint view_location, GLint view_pos_location, Camera *camera);
void camera_update(Camera* camera);
void camera_movement(Camera* camera, vec2s movement_axis, float delta_time);
void handle_mouse(Camera* camera, vec2s offsets, bool constrainPitch);
void update_camera_settings(Camera* camera, mat4 projection, GLint projection_location, float speed, float sensitivity, float fov);
void camera_ubo_init(GLuint *camera_ubo);
void upload_camera_ubo(GLuint camera_ubo, Camera *camera, mat4 projection);