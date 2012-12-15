
#pragma once

void sceneLoad(char* sceneFile);
void sceneDestroy();
void sceneDoLine(char* line, size_t lenLine);

struct scene_t
{
};

extern scene_t* g_theScene;
