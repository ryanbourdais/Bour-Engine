#include <stdio.h>
#include <stdlib.h>

#include "editor/editor_app.h"

int main(int argc, char **argv)
{
    if (argc > 2)
    {
        fprintf(stderr, "Usage: bour_editor [project-root]\n");
        return EXIT_FAILURE;
    }

    return editor_app_run(false, false, false, argc == 2 ? argv[1] : NULL);
}

