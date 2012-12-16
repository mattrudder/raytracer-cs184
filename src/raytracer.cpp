#include <stdio.h>
#include <stdlib.h>

#include <FreeImage.h>

#include "scene.h"

void printUsage(char* exeName);

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    FreeImage_Initialise();

    sceneLoad(argv[1]);

    if (g_theScene == NULL)
    {
        printf("Scene not loaded!");
    }

    sceneRender();
    sceneDestroy();

    FreeImage_DeInitialise();

    return 0;
}

void printUsage(char* exeName)
{
    printf("%s: Raytracer for CS184\n", exeName);
    printf("usage: %s sceneFile", exeName);
}


