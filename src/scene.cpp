
#include <stdlib.h>
#include <stdio.h>

#include "scene.h"

scene_t* g_theScene = NULL;

void sceneLoad(char* sceneFile)
{
    if (g_theScene)
        sceneDestroy();

    g_theScene = (scene_t*) malloc(sizeof(scene_t));
    
    FILE* file = fopen(sceneFile, "r");
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1)
    {
        sceneDoLine(line, len);
    }
    
    if (line)
        free(line);

    fclose(file);
}

void sceneDestroy()
{
    free(g_theScene);
    g_theScene = NULL;
}

void sceneDoLine(char* line, size_t lenLine)
{

}
