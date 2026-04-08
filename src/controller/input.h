#pragma once
#include <cglm/struct.h>

vec2s input_get_movement_axis();
vec2s input_get_movement_axis_smooth(float speed, float delta_time);
vec2s input_get_mouse_offsets(double xpos, double ypos);