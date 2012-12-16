
#pragma once

#include <glm/glm.hpp>

#include "scene.h"

struct ray_t
{
    glm::vec3 origin;
    glm::vec3 direction;
};

bool intersectRayTriangle(ray_t& ray, glm::vec3& a, glm::vec3& b, glm::vec3& c, float& u, float& v, float& w, float& t);
bool intersectRaySphere(ray_t& ray, sphere_t& sphere);
