
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <functional>
#include <vector>
#include <stack>

#include <FreeImage.h>

#include "utils.h"
#include "scene.h"
#include "ray.h"

#include <glm/gtc/matrix_transform.hpp>

scene_t* g_theScene = NULL;

struct scene_cmd_t
{
    const char* name;
    int arg_count;
    std::function<void(float *)> callback;
};

static int current_vertex = 0;
static int current_normal = 0;

static std::vector<triangle_t> triangles;
static std::vector<sphere_t> spheres;
static std::vector<light_t> lights;

static glm::mat4x4 xform;
static std::stack<glm::mat4x4> matrix_stack;

static const int c_bpp = 3;

static scene_cmd_t commands[] = 
{
    { "size", 2, [](float* args)
        {
           g_theScene->output_size.x = args[0];
           g_theScene->output_size.y = args[1];

           printf("Scene size: %gx%g\n", args[0], args[1]);
        }
    },
    { "maxdepth", 1, [](float* args)
        {
            g_theScene->max_depth = args[0];
        }
    },
    { "camera", 10, [](float* args)
        {
            camera_t& cam = g_theScene->camera;
            cam.look_from = glm::vec3(args[0], args[1], args[2]);
            cam.look_at = glm::vec3(args[3], args[4], args[5]);
            cam.up = glm::vec3(args[6], args[7], args[8]);
            cam.fov = glm::radians(args[9]);
        }
    },
    { "sphere", 4, [](float* args)
        {
            sphere_t s;
            s.position = glm::vec3(args[0], args[1], args[2]);
            s.radius = args[3];

            spheres.push_back(s);
        }
    },
    { "maxverts", 1, [](float* args)
        {
            int vcount = (int)args[0];
            g_theScene->vertex_count = vcount;
            g_theScene->vertices = new glm::vec3[vcount];
        }
    },
    { "maxvertnorms", 1, [](float* args)
        {
            int ncount = (int)args[0];
            g_theScene->normal_count = ncount;
            g_theScene->normals = new normal_t[ncount];
        }
    },
    { "vertex", 3, [](float* args)
        {
            g_theScene->vertices[current_vertex] = 
                glm::vec3(args[0], args[1], args[2]);

            current_vertex++;
        }
    },
    { "vertexnormal", 6, [](float* args)
        {
            g_theScene->normals[current_normal].position = 
                glm::vec3(args[0], args[1], args[2]);
            g_theScene->normals[current_normal].normal =
                glm::vec3(args[3], args[4], args[5]);

            current_normal++;
        }
    },
    { "tri", 3, [](float* args)
        {
            triangle_t t;
            t.indicies[0] = (int)args[0];
            t.indicies[1] = (int)args[1];
            t.indicies[2] = (int)args[2];

            triangles.push_back(t);
        }
    },
    { "trinormal", 3, [](float* args)
        {
            // TODO: Implement. Not used in test scenes.
        }
    },
    { "translate", 3, [](float* args)
        {
            xform = glm::translate(xform, glm::vec3(args[0], args[1], args[2]));
        }
    },
    { "rotate", 4, [](float* args)
        {
            xform = glm::rotate(xform, args[3], glm::vec3(args[0], args[1], args[2]));
        }
    },
    { "scale", 3, [](float* args)
        {
            xform = glm::scale(xform, glm::vec3(args[0], args[1], args[2]));
        }
    },
    { "pushTransform", 0, [](float* args)
        {
            matrix_stack.push(xform);
        }
    },
    { "popTransform", 0, [](float* args)
        {
            xform = matrix_stack.top();
            matrix_stack.pop();
        }
    },
    { "directional", 6, [](float* args)
        {
            light_t l;
            l.is_directional = true;
            l.position = glm::vec3(args[0], args[1], args[2]);
            l.color = glm::vec3(args[3], args[4], args[5]);
        }
    },
    { "point", 6, [](float* args)
        {
            light_t l;
            l.is_directional = false;
            l.position = glm::vec3(args[0], args[1], args[2]);
            l.color = glm::vec3(args[3], args[4], args[5]);
        }
    },
    { "attenuation", 3, [](float* args)
        {
        }
    },
    { "ambient", 3, [](float* args)
        {
        }
    },
    { "diffuse", 3, [](float* args)
        {
        }
    },
    { "specular", 3, [](float* args)
        {
        }
    },
    { "shininess", 1, [](float* args)
        {
        }
    },
    { "emission", 3, [](float* args)
        {
        }
    }
};

void sceneLoad(char* sceneFile)
{
    if (g_theScene)
        sceneDestroy();

    printf("Reading scene \"%s\"...\n", sceneFile); 

    g_theScene = new scene_t();
    
    FILE* file = fopen(sceneFile, "r");
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1)
        sceneDoLine(line, len);
    
    if (line)
        free(line);

    fclose(file);

    // copy scene objects into final location.
    size_t count = triangles.size();
    if (count > 0)
    {
        g_theScene->triangles = new triangle_t[count];
        g_theScene->triangle_count = count;
        memcpy(g_theScene->triangles, &triangles.at(0), sizeof(triangle_t) * count);
    }
    
    count = spheres.size();
    if (count > 0)
    {
        g_theScene->spheres = new sphere_t[spheres.size()];
        g_theScene->sphere_count = spheres.size();
        memcpy(g_theScene->spheres, &spheres.at(0), sizeof(sphere_t) * spheres.size());
    }
    
    count = lights.size();
    if (count > 0)
    {
        g_theScene->lights = new light_t[lights.size()];
        g_theScene->light_count = lights.size();
        memcpy(g_theScene->lights, &lights.at(0), sizeof(light_t) * lights.size());
    }

    printf("Scene information:\nTriangles: %ld, Spheres: %ld, Lights: %ld\n", triangles.size(), spheres.size(), lights.size());
}

void sceneDestroy()
{
    free(g_theScene);
    g_theScene = NULL;
}

void sceneDoLine(char* line, size_t lenLine)
{
    if (isEmpty(line))
        return;

    line = trim(line);

    if (*line == '\0' || *line == '#')          // comment/empty line
    {
        return;
    }
    else if (strncmp("output", line, 6) == 0)
    {
        char output[128] = {0};
        sscanf(line, "output %127s", (char*)&output);
        printf("Output set to %s\n", output);
        strncpy(g_theScene->output, output, 128);
    }
    else
    {
        for (scene_cmd_t cmd : commands)
        {
            size_t nameLen = strlen(cmd.name);
            if (strncmp(cmd.name, line, nameLen) == 0)
            {
                float* args = (float*) malloc(sizeof(float) * cmd.arg_count);
                char* strargs = line + nameLen;

                int i = 0;
                for (; i < cmd.arg_count && strargs != NULL && *strargs != '\0'; ++i)
                    args[i] = strtof(strargs, &strargs);

                cmd.callback(args);
                
                free(args);
            }
        }
    }
}

void sceneRender()
{
    // TODO: Save scene pixels as png
    int width = g_theScene->output_size.x;
    int height = g_theScene->output_size.y;
    const char* output = isEmpty(g_theScene->output) ? "out.png" : g_theScene->output;
    unsigned char* pixels = (unsigned char*) malloc(c_bpp * width * height);

    camera_t& cam = g_theScene->camera;
    float halfFov = cam.fov / 2;
    float tanFov = tan(halfFov);
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    glm::vec3 w = glm::normalize(cam.look_from - cam.look_at);
    glm::vec3 u = glm::normalize(glm::cross(cam.up, w));
    glm::vec3 v = glm::normalize(glm::cross(u, w));

    ray_t ray;
    ray.origin = cam.look_from;

    float multi = tanFov / halfHeight;
    unsigned char* currentPixel = pixels;
    for (int y = 0; y < height; ++y)
    {
        float cy = halfHeight - (y + 0.5f);
        for (int x = 0; x < width; currentPixel += c_bpp, ++x)
        {
            // TODO: Get Ray through pixel.
            float cx = (x + 0.5f) - halfWidth;
            float a = cx * multi;
            float b = cy * multi;

            ray.direction = glm::normalize((a * u) + (b * v) - w);

            // TODO: Find intersection with scene.
            glm::vec3* vertices = g_theScene->vertices;
            triangle_t* tri = g_theScene->triangles;
            for (int t = 0; t < g_theScene->triangle_count; ++t, ++tri)
            {
                int* indices = tri->indicies;
                float bu, bv, bw, bt;
                if (intersectRayTriangle(ray, vertices[indices[0]], vertices[indices[1]], vertices[indices[2]], bu, bv, bw, bt))
                {
                    // TODO: Shade pixel correctly.
                    *(currentPixel + 2) = 0xFF;
                }
            }
        }
    }

    printf("Rendering scene to %s...\n", output);
    FIBITMAP *img = FreeImage_ConvertFromRawBits(pixels, width, height, width * c_bpp, c_bpp * 8, 0xFF0000, 0x00FF00, 0x0000FF, false);
    FreeImage_Save(FIF_PNG, img, isEmpty(g_theScene->output) ? "out.png" : g_theScene->output, 0);

    free(pixels);

    printf("Render complete!\n");
}
