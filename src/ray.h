
#pragma once

#include <glm/glm.hpp>

#include "scene.h"

struct ray_t
{
    glm::vec3 origin;
    glm::vec3 direction;
};

struct ray_query_t
{
    object_t* obj;
    glm::vec3 pt;
    float u;
    float v;
    float w;
    float t;
};

bool operator <(const ray_query_t& lhs, const ray_query_t& rhs);
bool operator >(const ray_query_t& lhs, const ray_query_t& rhs);

bool intersectRayTriangle(ray_t& ray, glm::vec3& a, glm::vec3& b, glm::vec3& c, ray_query_t& query);
bool intersectRaySphere(ray_t& ray, sphere_t& sphere, ray_query_t& query);
