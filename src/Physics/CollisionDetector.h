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
        bool check(AABB &box, HitSphere &sphere);
};


#endif