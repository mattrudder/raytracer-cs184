
#pragma once

#include <memory>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

void sceneLoad(char* sceneFile);
void sceneDestroy();
void sceneDoLine(char* line, size_t lenLine);
void sceneRender();

struct camera_t
{
    glm::vec3 look_from;
    glm::vec3 look_at;
    glm::vec3 up;
    float fov;
};

struct material_t
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 emission;
    float shininess;
};

struct object_t
{
    // MEGA-HACKS
    bool is_sphere;
    glm::mat4x4 xform;
    glm::mat4x4 xform_inv;
    material_t material;
};

struct light_t
{
    bool is_directional;
    glm::vec3 position;
    glm::vec3 color;
};

struct normal_t
{
    glm::vec3 position;
    glm::vec3 normal;
};

struct triangle_t : public object_t
{
    int indicies[3];
};

struct sphere_t : public object_t
{
    glm::vec3 position;
    float radius;
};

struct scene_t
{
    char output[128];
    camera_t camera;
    glm::vec2 output_size;
    float max_depth;

    int vertex_count;
    int normal_count;
    int triangle_count;
    int sphere_count;
    int light_count;

    glm::vec3* vertices;
    normal_t* normals;
    triangle_t* triangles;
    sphere_t* spheres;
    light_t* lights;

    scene_t()
        : max_depth(5)
        , vertex_count(0)
        , vertices(NULL)
        , normal_count(0)
        , normals(NULL)
        , triangle_count(0)
        , triangles(NULL)
        , sphere_count(0)
        , spheres(NULL)
        , light_count(0)
        , lights(NULL)
    {
        memset(output, 0, 128);
        output_size = glm::vec2(320, 240);
    }
};

extern scene_t* g_theScene;
