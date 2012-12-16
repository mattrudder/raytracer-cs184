
#include "ray.h"

bool operator <(const ray_query_t& lhs, const ray_query_t& rhs)
{ 
    return lhs.t < rhs.t;
}

bool operator >(const ray_query_t& lhs, const ray_query_t& rhs)
{ 
    return lhs.t > rhs.t;
}

bool intersectRayTriangle(ray_t& ray, glm::vec3& a, glm::vec3& b, glm::vec3& c, ray_query_t& query)
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
    query.t = -(dot(normal, ray.origin) + d) / dr;
    if (query.t < 0) return false;

    vec3 p = ray.origin + (ray.direction * query.t);
    vec3 pa = p - a;
    query.v = dot(normal, cross(ba, pa));
    if (query.v < 0) return false;

    vec3 pb = p - b;
    vec3 cb = c - b;
    query.w = dot(normal, cross(cb, pb));
    if (query.w < 0) return false;

    vec3 pc = p - c;
    vec3 ac = a - c;
    query.u = dot(normal, cross(ac, pc));
    if (query.u < 0) return false;

    float nlen2 = dot(normal, normal);
    query.u /= nlen2;
    query.v /= nlen2;
    query.w /= nlen2;

    query.pt = ray.origin + query.t * ray.direction;

    return true;
}

bool intersectRaySphere(ray_t& ray, sphere_t& sphere, ray_query_t& query)
{
    using glm::dot;
    using glm::vec3;

    vec3 d = ray.direction;
    float t1 = -1;
    float t2 = -1;
    float discriminent;
    float t = -1;

    vec3 ec = ray.origin - sphere.position;
    float b = 2 * dot(d, ec);
    float a = dot(d, d);
    float c = dot(ec, ec) - (sphere.radius * sphere.radius);
    float disc = b * b - 4 * a * c;
    
    if (disc < 0) return false;

    discriminent = sqrt(disc);
    t1 = (-b + discriminent) / (2 * a);
    t2 = (-b - discriminent) / (2 * a);

    if (t1 > 0)
    {
        query.t = t2 > 0 ? glm::min(t1, t2) : t1;
        return true;
    }
    else if (t2 > 0)
    {
        query.t = t2;
        return true;
    }

    return false;
}
