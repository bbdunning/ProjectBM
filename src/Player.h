#include <memory>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "InputHandler.h"
#include "Physics/Hitbox.h"
#include "Physics/CollisionDetector.h"

class Player {
public:
    glm::vec3 velocity;
    glm::vec3 location;
    glm::vec3 lookAtPoint;
    std::shared_ptr<InputHandler> ih;
    std::shared_ptr<CollisionDetector> cd;
    bool isGrounded;
    bool standing;
    bool hasDoubleJump;
    bool facingRight;
    bool isAttacking = false;
    float theta;
    float phi;
    float prevX;
    float prevY;
    HitSphere environmentalHbox;

    Player();
    int init(std::shared_ptr<InputHandler> ih);
    int update(float dt);
    void setViewAngles(GLFWwindow *window);
    glm::vec3 getForwardDir();
    glm::vec3 getRightDir();
};


class Sandbag{
public:
    glm::vec3 velocity;
    glm::vec3 location;
    std::shared_ptr<InputHandler> ih;
    std::shared_ptr<CollisionDetector> cd;
    bool isGrounded;
    bool standing;
    bool hasDoubleJump;
    bool facingRight;
    HitSphere environmentalHbox;
    float currentPercent = 0;
    bool isHitstun = false;

    Sandbag();
    int init(std::shared_ptr<InputHandler> ih);
    int update(std::vector<HitSphere> &hitboxes);
    int checkCollisions(std::vector<HitSphere> &hitboxes);
};