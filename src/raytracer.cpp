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

    sceneDestroy();

    // TODO: Save scene pixels as png
    //FIBITMAP *img = FreeImage_ConvertFromRawBits(pixels, w, h, w * 3, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);
    //FreeImage_Save(FIF_PNG, img, "out.png", 0);

    FreeImage_DeInitialise();

    return 0;
}

void printUsage(char* exeName)
{
    printf("%s: Raytracer for CS184\n", exeName);
    printf("usage: %s sceneFile", exeName);
}


