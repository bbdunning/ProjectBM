#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "WindowManager.h"
#include "Player.h"
#include "InputHandler.h"

using namespace glm;
using namespace std;

//need to change
#define MAX_GRAVITY 100
#define MAX_SPEED 1

Player::Player() {
}

int Player::init(shared_ptr<InputHandler> ih) {
    this->velocity=vec3(0,0,0);
    this->location=vec3(0,-.7,-2);
    this->ih = ih;
    this->isGrounded = true;
    this->hasDoubleJump = true;
    return 0;
}

int Player::update() {
    // int axesCount;
    // const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
    // std::cout << "Left Stick X Axis: " << axes[0] << std::endl;

    if (location.y <= -.7) {
        isGrounded = true;
        velocity.y = 0;
        hasDoubleJump = true;
    }
    if (!isGrounded && velocity.y < MAX_GRAVITY)
    velocity.y -= .002;
    if (ih->Spaceflag && isGrounded) {
        // velocity.y += .065; //fullhop
        velocity.y += .04; //shorthop
        isGrounded=false;
    }
    
    //double jump
    if (ih->Upflag && !isGrounded && hasDoubleJump) {
        velocity.y = .06;
        hasDoubleJump = false;
    }
    
    //grounded movement
    if (ih->Leftflag && velocity.x > -MAX_SPEED && isGrounded)
        // if (velocity.x > -.01  && velocity.x < .01)
        //     velocity.x = -.03;
        // else
            velocity.x -= .001;
    if (ih->Rightflag && velocity.x < MAX_SPEED && isGrounded)
        velocity.x += .001;
    
    //friction
    if ((!ih->Leftflag or !isGrounded) && velocity.x < 0)
        velocity.x += .002;
    if ((!ih->Rightflag  or !isGrounded) && velocity.x > 0)
        velocity.x -= .002;
    
    //controller
    // if (axes[0]==-1 && velocity.x > -MAX_SPEED)
    //     velocity.x -= .001;
    // if (axes[0]==1 && velocity.x < MAX_SPEED)
    //     velocity.x += .001;
    // if (!axes[0]==-1 && velocity.x < 0)
    //     velocity.x += .001;
    // if (!axes[0]==1 && velocity.x > 0)
    //     velocity.x -= .001;

    location += velocity;
    return 0;
}