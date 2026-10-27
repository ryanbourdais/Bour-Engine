#include "project_paths.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool project_reference_is_normalized(const char *reference)
{
    const char *segment = reference;

    for (const char *cursor = reference;  ; cursor++)
    {
        if (*cursor == '\\' || *cursor == ':')
        {
            return false;
        }

        if (*cursor == '/' || *cursor == '\0')
        {
            size_t length = (size_t)(cursor - segment);

            if (length == 0 ||
                (length == 1 && segment[0] == '.') ||
                (length == 2 && segment[0] == '.' && segment[1] == '.'))
            {
                return false;
            }

            if (*cursor == '\0')
            {
                return true;
            }

            segment = cursor + 1;
        }
    }
}

ProjectPathResult project_path_resolve(
    const char *project_root,
    const char *reference,
    char *out_path,
    size_t out_capacity
)
{
    if (out_path == NULL || out_capacity == 0)
    {
        return PROJECT_PATH_INVALID_ARGUMENT;
    }

    out_path[0] = '\0';

    if (project_root == NULL || project_root[0] == '\0' ||
        reference == NULL || reference[0] == '\0')
    {
        return PROJECT_PATH_INVALID_ARGUMENT;
    }

    if (!project_reference_is_normalized(reference))
    {
        return PROJECT_PATH_INVALID_REFERENCE;
    }

    size_t root_length = strlen(project_root);
    char last_root_character = project_root[root_length - 1];

    const char *separator =
        last_root_character == '/' || last_root_character == '\\'
            ? ""
            : "/";

    int written = snprintf(
        out_path, 
        out_capacity, 
        "%s%s%s",
        project_root,
        separator,
        reference
    );

    if (written < 0 || (size_t)written >= out_capacity)
    {
        out_path[0] = '\0';
        return PROJECT_PATH_TOO_LONG;
    }

    return PROJECT_PATH_OK;
}





