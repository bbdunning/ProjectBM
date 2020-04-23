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
#define MAX_SPEED .045f
#define MAX_AIR_SPEED .03f

Player::Player() {
}

int Player::init(shared_ptr<InputHandler> ih) {
    this->velocity=vec3(0,0,0);
    // this->location=vec3(0,-.7,-2);
    this->location=vec3(0,-1,-2);
    this->ih = ih;
    this->isGrounded = true;
    this->standing = true; //change to true
    this->hasDoubleJump = true;
    this->facingRight = true;
    return 0;
}

int Player::update() {
    // int axesCount;
    // const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
    // std::cout << "Left Stick X Axis: " << axes[0] << std::endl;

    if (location.y <= -1) {
        if (!isGrounded) {
            //landing lag
            location.y = -1;
            standing = true;
        }
        isGrounded = true;
        velocity.y = 0;
        hasDoubleJump = true;
    }
    if (!isGrounded && velocity.y < MAX_GRAVITY)
    velocity.y -= .005;
    if (ih->Spaceflag && isGrounded) {
        // velocity.y += .065; //fullhop
        velocity.y = .075; //shorthop
        velocity.x = clamp(velocity.x, -0.018f, 0.018f);
        isGrounded=false;
        standing = false;
    }
    
    //double jump
    if (ih->Upflag && !isGrounded && hasDoubleJump) {
        //double jump should be able to change velocity direction
        velocity.y = .05;
        hasDoubleJump = false;
    }
    
    //grounded movement
    if (ih->Leftflag && velocity.x > -MAX_SPEED && isGrounded) {
        if (standing) {
            velocity.x = -.02;
            facingRight = false;
        }
        else if (!facingRight)
            velocity.x -= .002;
        standing = false;
    }
    if (ih->Rightflag && velocity.x < MAX_SPEED && isGrounded)
    {
        if (standing) {
            velocity.x = .02;
            facingRight = true;
        }
        else if (facingRight)
            velocity.x += .002;
        standing = false;
    }

    //arial movment
    if (ih->Leftflag && (velocity.x > -MAX_AIR_SPEED) && !isGrounded)
        velocity.x -= .0006;
    if (ih->Rightflag && (velocity.x < MAX_AIR_SPEED) && !isGrounded)
        velocity.x += .0006;
    
    //grounded friction
    if (isGrounded && velocity.x < 0.0f && !ih->Leftflag && !standing)
    {
        if (velocity.x > -.009)
            standing = true;
        else
            velocity.x += .003f;
    }
    if (isGrounded && velocity.x > 0.0f && !ih->Rightflag && !standing)
    {
        if (velocity.x < .009)
            standing = true;
        else
            velocity.x -= .003;
    }

    if ((velocity.x > -.0005 && velocity.x < .0005) && isGrounded)
        standing = true;

    if (standing) velocity.x = 0.0f;
    
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