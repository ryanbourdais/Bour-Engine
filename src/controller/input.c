#include "input.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

float_vec2 input_get_movement_axis() {
    float_vec2 movement_axis = {0.0f, 0.0f};
    movement_axis.y += glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W);
    movement_axis.y -= glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S);
    movement_axis.x += glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D);
    movement_axis.x -= glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A);
    return movement_axis;
}

float_vec2 input_get_movement_axis_smooth(float speed, float delta_time) {
    float_vec2 movement_axis = {0.0f, 0.0f};
    movement_axis.y += glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) * speed * delta_time;
    movement_axis.y -= glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) * speed * delta_time;
    movement_axis.x += glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) * speed * delta_time;
    movement_axis.x -= glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) * speed * delta_time;
    return movement_axis;
}