#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "Program.h"

class Hitbox
{
    public:
        virtual bool checkCollision(Hitbox other);
};

class HitSphere
{
    public:
        vec3 center;
        float radius;

        bool checkCollision(Hitbox other) override;
}

class HitBox
{
    public:
        bool checkCollision(Hitbox other) override;
}