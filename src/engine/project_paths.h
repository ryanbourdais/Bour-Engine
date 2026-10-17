#pragma once

#include <stddef.h>

typedef enum ProjectPathResult {
    PROJECT_PATH_OK = 0,
    PROJECT_PATH_INVALID_ARGUMENT,
    PROJECT_PATH_INVALID_REFERENCE,
    PROJECT_PATH_TOO_LONG,
} ProjectPathResult;

ProjectPathResult project_path_resolve(
    const char *project_root,
    const char *reference,
    char *out_path,
    size_t out_capacity
);


