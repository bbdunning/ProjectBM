#pragma once
#ifndef HITBOX_H
#define HITBOX_H
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class HitSphere;
class AABB;

class Hitbox
{
    public:
        virtual bool checkCollision(HitSphere &sphere);
        virtual bool checkCollision(AABB &box);
};

//sphere-shaped hitbox
class HitSphere : public Hitbox
{
    public:
        glm::vec3 center;
        float radius;

        HitSphere() {};
        HitSphere(glm::vec3 center, float radius);
        bool checkCollision(HitSphere &sphere);
        bool checkCollision(AABB &box);
};

//Axis-Aligned Bounding Box (AABB)
class AABB : public Hitbox
{
    public:
        glm::vec3 min, max;

        AABB(glm::vec3 min, glm::vec3 max);
        // bool checkCollision(Hitbox other) override;
        bool checkCollision(HitSphere &sphere);
        bool checkCollision(AABB &box);
};


#endif