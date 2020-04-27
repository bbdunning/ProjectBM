#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class Hitbox
{
    public:
        virtual bool checkCollision(Hitbox other);
};

//sphere-shaped hitbox
class HitSphere : public Hitbox
{
    public:
        glm::vec3 center;
        float radius;

        HitSphere(glm::vec3 center, float radius);
        bool checkCollision(Hitbox other) override;
};

//Axis-Aligned Bounding Box (AABB)
class AABB : public Hitbox
{
    public:
        glm::vec3 min, max;

        bool checkCollision(Hitbox other) override;
};