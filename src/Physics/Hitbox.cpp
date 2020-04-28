#include "Hitbox.h"
#include <iostream>

using namespace std;
using namespace glm;

AABB::AABB(vec3 min, vec3 max) {
    this->min = min;
    this->max = max;
}

bool Hitbox::checkCollision(HitSphere &sphere) {
    return 0;
}
bool Hitbox::checkCollision(AABB &box) {
    return 0;
}

// bool AABB::checkCollision(Hitbox other) {
//     return 0;
// }

bool AABB::checkCollision(HitSphere &sphere) {
  // get box closest point to sphere center by clamping
    float x = glm::max(this->min.x, glm::min(sphere.center.x, this->max.x));
    float y = glm::max(this->min.y, glm::min(sphere.center.y, this->max.y));
    float z = glm::max(this->min.z, glm::min(sphere.center.z, this->max.z));

    // this is the same as isPointInsideSphere
    float distance = sqrt((x - sphere.center.x) * (x - sphere.center.x) +
                            (y - sphere.center.y) * (y - sphere.center.y) +
                            (z - sphere.center.z) * (z - sphere.center.z));

    // cout << " distance: " << distance << " radius: " << sphere.radius; 
    if (distance < sphere.radius)
        cout << "REEEEE" << endl;
    return distance < 1 + sphere.radius;
}

bool AABB::checkCollision(AABB &other) {
    return 0;
}

HitSphere::HitSphere(glm::vec3 center, float radius) {
    this->center = center;
    this->radius = radius;
}

bool HitSphere::checkCollision(AABB &box) {
    return 0;
    float x = glm::max(box.min.x, glm::min(this->center.x, box.max.x));
    float y = glm::max(box.min.y, glm::min(this->center.y, box.max.y));
    float z = glm::max(box.min.z, glm::min(this->center.z, box.max.z));

    // this is the same as isPointInsideSphere
    float distance = sqrt((x - this->center.x) * (x - this->center.x) +
                            (y - this->center.y) * (y - this->center.y) +
                            (z - this->center.z) * (z - this->center.z));

    // cout << " distance: " << distance << " radius: " << this->radius; 
    if (distance < this->radius)
        cout << "REEEEE" << endl;
    return distance < 1 + this->radius;
}

bool HitSphere::checkCollision(HitSphere &sphere) {
    return 0;
}
