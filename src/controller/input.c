#include "input.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

static bool firstMouse = true;
static int lastX = 0;
static int lastY = 0;

vec2s input_get_movement_axis() {
    vec2s movement_axis = {0.0f, 0.0f};
    movement_axis.y += glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W);
    movement_axis.y -= glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S);
    movement_axis.x += glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D);
    movement_axis.x -= glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A);
    return movement_axis;
}

vec2s input_get_movement_axis_smooth(float speed, float delta_time) {
    vec2s movement_axis = {0.0f, 0.0f};
    movement_axis.y += glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) * speed * delta_time;
    movement_axis.y -= glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) * speed * delta_time;
    movement_axis.x += glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) * speed * delta_time;
    movement_axis.x -= glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) * speed * delta_time;
    return movement_axis;
}


vec2s input_get_mouse_offsets(double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    
    vec2s offsets = {xOffset, yOffset};
    return offsets;
}