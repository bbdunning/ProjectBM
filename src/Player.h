#include <memory>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "InputHandler.h"
#include "Camera.h"

class Player {
public:
    glm::vec3 velocity;
    glm::vec3 location;
    std::shared_ptr<InputHandler> ih;
    bool isGrounded;
    bool standing;
    bool hasDoubleJump;
    bool facingRight;
    float moveVelocity;

    Player();
    int init(std::shared_ptr<InputHandler> ih);
    int update(Camera *camera);
};