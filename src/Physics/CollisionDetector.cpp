#include "CollisionDetector.h"

using namespace std;

bool CollisionDetector::check(AABB &box, HitSphere &sphere) {
  // get box closest point to sphere center by clamping
    float x = glm::max(box.min.x, glm::min(sphere.center.x, box.max.x));
    float y = glm::max(box.min.y, glm::min(sphere.center.y, box.max.y));
    float z = glm::max(box.min.z, glm::min(sphere.center.z, box.max.z));

    // this is the same as isPointInsideSphere
    float distance = sqrt((x - sphere.center.x) * (x - sphere.center.x) +
                            (y - sphere.center.y) * (y - sphere.center.y) +
                            (z - sphere.center.z) * (z - sphere.center.z));

    cout << " distance: " << distance << " radius: " << sphere.radius << endl;; 
    // if (distance < sphere.radius)
    //     cout << "REEEEE" << endl;
    return distance < sphere.radius;
}