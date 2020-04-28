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
#define MAX_SPEED .035f
#define MAX_AIR_SPEED .03f

Player::Player() {
}

int Player::init(shared_ptr<InputHandler> ih) {
    this->velocity=vec3(0,0,0);
    // this->location=vec3(0,-.7,-2);
    this->location=vec3(0,-1,-2);
    this->ih = ih;
    this->isGrounded = true;
    this->standing = true;
    this->hasDoubleJump = true;
    this->facingRight = true;

    this->environmentalHbox = HitSphere(this->location, .008);
    return 0;
}

int Player::update() {
    // int axesCount;
    // const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
    // std::cout << "Left Stick X Axis: " << axes[0] << std::endl;
    this->environmentalHbox.center = this->location;
    bool isOnPlatform = false;

    //landing
    // if (location.y <= -1) {
    //     if (!isGrounded) {
    //         //add landing lag
    //         location.y = -1;
    //         standing = true;
    //     }
    //     isGrounded = true;
    //     velocity.y = 0;
    //     hasDoubleJump = true;
    // }

    //landing
    for (int i=0; i<cd->environmentBoxes.size(); i++) {
        if (cd->check(*cd->environmentBoxes[i], this->environmentalHbox) && velocity.y <= 0 && !ih->Downflag) {// && !isGrounded) {
            if (!isGrounded) {
                standing=true;
            }
            isGrounded = true;
            velocity.y = 0;
            hasDoubleJump = true;
            isOnPlatform = true;
            this->location.y = cd->environmentBoxes[i]->max.y;
        } 
    }
    cout << cd->check2(*cd->environmentBoxes[3], this->environmentalHbox) << endl;
    if (cd->check2(*cd->environmentBoxes[3], this->environmentalHbox) && velocity.y <=0) {// && !isGrounded) {
        if (!isGrounded) {
            standing=true;
        }
        isGrounded = true;
        velocity.y = 0;
        hasDoubleJump = true;
        isOnPlatform = true;
        this->location.y = cd->environmentBoxes[3]->max.y;
    } 
    if (!isOnPlatform || ih->Downflag) {
        isGrounded = false;
    }


    //gravity
    if (!isGrounded && velocity.y < MAX_GRAVITY)
        velocity.y -= .004;

    //jump
    if (ih->Spaceflag && isGrounded) {
        // velocity.y += .065; //fullhop
        velocity.y = .07; //shorthop
        velocity.x = clamp(velocity.x, -0.018f, 0.018f);
        isGrounded = false;
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
        else if (facingRight) {
            velocity.x = -MAX_SPEED;
            facingRight = false;
        }
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
        else if (!facingRight) {
            velocity.x = MAX_SPEED;
            facingRight = true;
        }
        standing = false;
    }

    //arial movment
    if (ih->Leftflag && (velocity.x > -MAX_AIR_SPEED) && !isGrounded)
        velocity.x -= .0009;
    if (ih->Rightflag && (velocity.x < MAX_AIR_SPEED) && !isGrounded)
        velocity.x += .0009;
    
    //grounded friction
    if (isGrounded && velocity.x < 0.0f && !ih->Leftflag && !standing)
    {
        //stop and stand
        if (velocity.x > -.009)
            standing = true;
        //slow down
        else
            velocity.x += .002f;
    }
    if (isGrounded && velocity.x > 0.0f && !ih->Rightflag && !standing)
    {
        if (velocity.x < .009)
            standing = true;
        else
            velocity.x -= .002;
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
    // cout << location.x << " " << location.y << " " << location.z << endl;
    return 0;
}