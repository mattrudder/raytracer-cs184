
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <functional>
#include <vector>
#include <stack>
#include <queue>

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

static std::stack<glm::mat4> matrix_stack;

static const int c_bpp = 3;

static material_t material;

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
            s.is_sphere = true;
            s.position = glm::vec3(args[0], args[1], args[2]);
            s.radius = args[3];
            s.material = material;
            s.xform = matrix_stack.top();
            s.xform_inv = glm::inverse(s.xform);

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
            t.is_sphere = false;
            t.indicies[0] = (int)args[0];
            t.indicies[1] = (int)args[1];
            t.indicies[2] = (int)args[2];
            t.material = material;
            t.xform = matrix_stack.top();
            t.xform_inv = glm::inverse(t.xform);

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
            matrix_stack.top() = glm::translate(matrix_stack.top(), glm::vec3(args[0], args[1], args[2]));
        }
    },
    { "rotate", 4, [](float* args)
        {
            matrix_stack.top() = glm::rotate(matrix_stack.top(), args[3], glm::vec3(args[0], args[1], args[2]));
        }
    },
    { "scale", 3, [](float* args)
        {
            matrix_stack.top() = glm::scale(matrix_stack.top(), glm::vec3(args[0], args[1], args[2]));
        }
    },
    { "pushTransform", 0, [](float* args)
        {
            matrix_stack.push(matrix_stack.top());
        }
    },
    { "popTransform", 0, [](float* args)
        {
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
            material.ambient = glm::vec3(args[0], args[1], args[2]);
        }
    },
    { "diffuse", 3, [](float* args)
        {
            material.diffuse = glm::vec3(args[0], args[1], args[2]);
        }
    },
    { "specular", 3, [](float* args)
        {
            material.specular = glm::vec3(args[0], args[1], args[2]);
        }
    },
    { "shininess", 1, [](float* args)
        {
            material.shininess = args[0];
        }
    },
    { "emission", 3, [](float* args)
        {
            material.emission = glm::vec3(args[0], args[1], args[2]);
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

    // default scene attributes
    material.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    material.diffuse = glm::vec3(0, 0, 0);
    material.specular = glm::vec3(0, 0, 0);
    material.emission = glm::vec3(0, 0, 0);
    material.shininess = 0;

    matrix_stack.push(glm::mat4(1.0));

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
    using glm::cross;
    using glm::clamp;
    using glm::vec3;
    using glm::normalize;

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

    vec3 w = normalize(cam.look_from - cam.look_at);
    vec3 u = normalize(cross(cam.up, w));
    vec3 v = normalize(cross(u, w));

    ray_t ray;
    ray.origin = cam.look_from;

    ray_query_t best;

    float multi = tanFov / halfHeight;
    unsigned char* currentPixel = pixels;
    for (int y = 0; y < height; ++y)
    {
        float cy = halfHeight - (y + 0.5f);
        for (int x = 0; x < width; currentPixel += c_bpp, ++x)
        {
            // reset best query.
            best.obj = NULL;
            best.t = FLT_MAX;

            // Get Ray through pixel.
            float cx = (x + 0.5f) - halfWidth;
            float a = cx * multi;
            float b = cy * multi;

            vec3 rayDirection = normalize((a * u) + (b * v) - w);

            // Find intersection with scene.
            ray_query_t query;
            vec3* vertices = g_theScene->vertices;
            triangle_t* tri = g_theScene->triangles;
            for (int t = 0; t < g_theScene->triangle_count; ++t, ++tri)
            {
                glm::vec4 rayOriginT = tri->xform_inv * glm::vec4(cam.look_from, 1.0f);
                glm::vec4 rayDirectionT = tri->xform_inv * glm::vec4(rayDirection, 0.0f);

                ray.origin = vec3(rayOriginT.x, rayOriginT.y, rayOriginT.z);
                ray.direction = vec3(rayDirectionT.x, rayDirectionT.y, rayDirectionT.z);

                int* indices = tri->indicies;
                if (intersectRayTriangle(ray, vertices[indices[0]], vertices[indices[1]], vertices[indices[2]], query))
                {
                    query.obj = tri;
                    if (query.t < best.t) best = query;
                }
            }

            sphere_t* sph = g_theScene->spheres;
            for (int s = 0; s < g_theScene->sphere_count; ++s, ++sph)
            {
                glm::vec4 rayOriginT = sph->xform_inv * glm::vec4(cam.look_from, 1.0f);
                glm::vec4 rayDirectionT = sph->xform_inv * glm::vec4(rayDirection, 0.0f);

                ray.origin = vec3(rayOriginT.x, rayOriginT.y, rayOriginT.z);
                ray.direction = vec3(rayDirectionT.x, rayDirectionT.y, rayDirectionT.z);
                
                if (intersectRaySphere(ray, *sph, query))
                {
                    query.obj = sph;
                    if (query.t < best.t) best = query;
                }
            }

            // TODO: Light object
            if (best.obj != NULL)
            {
                material_t& mat = best.obj->material;
                vec3 color = mat.ambient;

                // final color conversion.
                currentPixel[0] = (unsigned char) (clamp(color.b, 0.0f, 1.0f) * 255.0f);
                currentPixel[1] = (unsigned char) (clamp(color.g, 0.0f, 1.0f) * 255.0f);
                currentPixel[2] = (unsigned char) (clamp(color.r, 0.0f, 1.0f) * 255.0f);
            }
        }
    }

    printf("Rendering scene to %s...\n", output);
    FIBITMAP *img = FreeImage_ConvertFromRawBits(pixels, width, height, width * c_bpp, c_bpp * 8, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, false);
    FreeImage_Save(FIF_PNG, img, isEmpty(g_theScene->output) ? "out.png" : g_theScene->output, 0);

    free(pixels);

    printf("Render complete!\n");
}
