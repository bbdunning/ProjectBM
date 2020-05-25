#include <memory>
#include <iostream>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/projection.hpp>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/LinearMath/btAabbUtil2.h>

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
    float projectileCooldown = 0;
    HitSphere environmentalHbox;
    // btRigidBody *playerBody;

    Player();
    int init(std::shared_ptr<InputHandler> ih);
    int update(float dt);
    void setViewAngles(GLFWwindow *window);
    glm::vec3 getForwardDir();
    glm::vec3 getRightDir();
    glm::vec3 getForwardMoveDir();
    float getFacingAngle();
    void move(float dt, btRigidBody *playerBody, btDynamicsWorld *dynamicsWorld);
    void updateLocation(btRigidBody *playerBody);
    // float getForwardSpeed();
};