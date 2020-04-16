#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Player.h"
#include "InputHandler.h"

using namespace glm;
using namespace std;

Player::Player() {
    this->velocity=vec3(0,0,0);
    this->location=vec3(0,0,0);
}

// int Player::init(shared_ptr<InputHandler> ih) {
//     this->ih = ih;

//     return 0;
// }

int Player::update() {
    velocity.y -= .001;
    // if (ih->Spaceflag) {
    //     velocity.y += 10;
    // }

    location += velocity;
    return 0;
}