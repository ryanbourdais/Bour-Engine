#pragma once

#include <cglm/struct.h>

#ifdef __cplusplus
extern "C" {
#endif

void copy_float3(float dest[3], const float src[3]);
void copy_vec3_rgb_to_float3(float dest[3], const vec3s src);
void copy_vec3_xyz_to_float3(float dest[3], const vec3s src);
void copy_float3_to_vec3_xyz(vec3s *dest, float src[3]);

#ifdef __cplusplus
}
#endif
