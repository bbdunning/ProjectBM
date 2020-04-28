#pragma once
#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <iostream>
#include "Hitbox.h"

class CollisionDetector
{
    public:
        CollisionDetector() {};
        std::vector<std::shared_ptr<AABB>> environmentBoxes;
        static bool check(AABB &box, HitSphere &sphere);
        static bool check2(AABB &box, HitSphere &sphere);
        static bool sphereCheck(HitSphere s1, HitSphere s2);
};


#endif