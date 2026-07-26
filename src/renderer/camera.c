#include "camera.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

void camera_init(Camera *camera)
{
    camera->cameraPos = (vec3s){0.0f, 0.0f, 3.0f};

    camera->cameraTarget = (vec3s){0.0f, 0.0f, 0.0f};

    camera->cameraDirection = glms_vec3_normalize(glms_vec3_sub(camera->cameraPos, camera->cameraTarget));

    vec3s up = (vec3s){0.0f, 1.0f, 0.0f};
    camera->cameraRight = glms_vec3_normalize(glms_vec3_cross(up, camera->cameraDirection));

    camera->cameraUp = glms_vec3_cross(camera->cameraDirection, camera->cameraRight);
    camera->cameraFront = (vec3s){0.0f, 0.0f, -1.0f};

    camera->cameraSpeed = 2.5f;
    camera->cameraSensitivity = 0.1f;
    camera->cameraYaw = -90.0f;
    camera->cameraPitch = 0.0f;
    camera->cameraFOV = 45.0f;
}

void upload_camera(GLint view_location, GLint view_pos_location, Camera *camera)
{
    glUniformMatrix4fv(
        view_location,
        1,
        GL_FALSE,
        (float *)camera->view.raw
    );

    glUniform3fv(
        view_pos_location,
        1,
        camera->cameraPos.raw
    );
}

void update_camera_settings(Camera *camera, mat4 projection, GLint projection_location, float speed, float sensitivity, float fov)
{
    camera->cameraSpeed = speed;
    camera->cameraSensitivity = sensitivity;
    camera->cameraFOV = fov;
    glm_perspective(glm_rad(camera->cameraFOV), 800.0f / 600.0f, 0.1f, 100.0f, projection);
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, (float *)projection);
}

void camera_update(Camera *camera)
{
    camera->view = glms_lookat(
        camera->cameraPos,
        glms_vec3_add(camera->cameraPos, camera->cameraFront),
        camera->cameraUp);
}

void camera_movement(Camera *camera, vec2s movement_axis, float delta_time)
{
    float velocity = camera->cameraSpeed * delta_time;
    vec3s right = glms_vec3_normalize(glms_vec3_cross(camera->cameraFront, camera->cameraUp));
    if (movement_axis.y > 0.0f)
    {
        camera->cameraPos = glms_vec3_add(
            camera->cameraPos,
            glms_vec3_scale(camera->cameraFront, velocity));
    }

    if (movement_axis.y < 0.0f)
    {
        camera->cameraPos = glms_vec3_sub(
            camera->cameraPos,
            glms_vec3_scale(camera->cameraFront, velocity));
    }

    if (movement_axis.x < 0.0f)
    {
        camera->cameraPos = glms_vec3_sub(
            camera->cameraPos,
            glms_vec3_scale(right, velocity));
    }

    if (movement_axis.x > 0.0f)
    {
        camera->cameraPos = glms_vec3_add(
            camera->cameraPos,
            glms_vec3_scale(right, velocity));
    }
}

void handle_mouse(Camera *camera, vec2s offsets, bool constrainPitch)
{
    float xOffset = offsets.x;
    float yOffset = offsets.y;

    xOffset *= camera->cameraSensitivity;
    yOffset *= camera->cameraSensitivity;

    camera->cameraYaw += xOffset;
    camera->cameraPitch += yOffset;

    if (constrainPitch)
    {
        if (camera->cameraPitch > 89.0f)
            camera->cameraPitch = 89.0f;
        if (camera->cameraPitch < -89.0f)
            camera->cameraPitch = -89.0f;
    }

    vec3s direction;
    direction.x = cos(glm_rad(camera->cameraYaw)) * cos(glm_rad(camera->cameraPitch));
    direction.y = sin(glm_rad(camera->cameraPitch));
    direction.z = sin(glm_rad(camera->cameraYaw)) * cos(glm_rad(camera->cameraPitch));
    camera->cameraFront = glms_normalize(direction);
}
/* Spin camera:
const float radius = 10.0f;
    float camX = sin(glfwGetTime()) * radius;
    float camZ = cos(glfwGetTime()) * radius;

    camera->cameraPos = (vec3s){camX, 0.0f, camZ};
    camera->view = glms_lookat((vec3s){camX, 0.0, camZ},
                              (vec3s){0.0f, 0.0f, 0.0f},
                              (vec3s){0.0f, 1.0f, 0.0f});*/

void camera_ubo_init(GLuint *camera_ubo)
{
    glGenBuffers(1, camera_ubo);

    glBindBuffer(GL_UNIFORM_BUFFER, *camera_ubo);

    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraUniformData), NULL, GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA_UBO_BINDING, *camera_ubo);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void upload_camera_ubo(GLuint camera_ubo, const Camera *camera, mat4 projection)
{
    CameraUniformData data = {0};

    glm_mat4_copy(camera->view.raw, data.view);
    glm_mat4_copy(projection, data.projection);

    data.view_pos[0] = camera->cameraPos.x;
    data.view_pos[1] = camera->cameraPos.y;
    data.view_pos[2] = camera->cameraPos.z;
    data.view_pos[3] = 1.0f;

    glBindBuffer(GL_UNIFORM_BUFFER, camera_ubo);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraUniformData), &data);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}