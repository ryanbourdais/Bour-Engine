#include "math_utils.h"

void copy_float3(float dest[3], const float src[3])
{
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];
}

void copy_vec3_xyz_to_float3(float dest[3], const vec3s src)
{
    dest[0] = src.x;
    dest[1] = src.y;
    dest[2] = src.z;
}

void copy_vec3_rgb_to_float3(float dest[3], const vec3s src)
{
    dest[0] = src.r;
    dest[1] = src.g;
    dest[2] = src.b;
}