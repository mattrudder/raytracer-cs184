
#include "ray.h"

bool intersectRayTriangle(ray_t& ray, glm::vec3& a, glm::vec3& b, glm::vec3& c, float& u, float& v, float& w, float& t)
{
    using glm::vec3;
    using glm::dot;
    using glm::cross;

    vec3 ba = b - a;
    vec3 ca = c - a;
    vec3 normal = cross(ba, ca);
    float dr = dot(normal, ray.direction);
    
    // TODO: Handle single sided?
    if (dr == 0) return false;

    float d = dot(normal, a);
    t = -(dot(normal, ray.origin) + d) / dr;
    if (t < 0) return false;

    vec3 p = ray.origin + (ray.direction * t);
    vec3 pa = p - a;
    v = dot(normal, cross(ba, pa));
    if (v < 0) return false;

    vec3 pb = p - b;
    vec3 cb = c - b;
    w = dot(normal, cross(cb, pb));
    if (w < 0) return false;

    vec3 pc = p - c;
    vec3 ac = a - c;
    u = dot(normal, cross(ac, pc));
    if (u < 0) return false;

    float nlen2 = dot(normal, normal);
    u /= nlen2;
    v /= nlen2;
    w /= nlen2;

    return true;
}

bool intersectRaySphere(ray_t& ray, sphere_t& sphere)
{
    return false;
}
