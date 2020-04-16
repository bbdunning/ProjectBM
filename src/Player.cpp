#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Player.h"

using namespace glm;

Player::Player() {
    this->velocity=vec3(0,0,0);
    this->location=vec3(0,0,0);
}

int Player::update() {
    velocity.y -= .1;

    location -= velocity;
    return 0;
}