#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "WindowManager.h"
#include "Player.h"
#include "InputHandler.h"
#include "Camera.h"

using namespace glm;
using namespace std;

//need to change
#define MAX_GRAVITY 100
#define MAX_SPEED .045f
#define MAX_AIR_SPEED .03f
#define GROUND 0.0f

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
    this->moveVelocity = .4;
    return 0;
}

int Player::update(Camera *camera) {
    // int axesCount;
    // const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
    // std::cout << "Left Stick X Axis: " << axes[0] << std::endl;

    if (camera->eye.y <= GROUND) {
        if (!isGrounded) {
            //landing lag
            camera->eye.y = GROUND;
            standing = true;
        }
        isGrounded = true;
        velocity.y = 0;
        hasDoubleJump = true;
    }
    //gravity
    if (!isGrounded && velocity.y < MAX_GRAVITY)
        velocity.y -= .005;
    //jump
    if (ih->Spaceflag && isGrounded) {
        // velocity.y += .065; //fullhop
        velocity.y = .075; //shorthop
        velocity.x = clamp(velocity.x, -0.018f, 0.018f);
        isGrounded=false;
        standing = false;
    }
    
    // //double jump
    // if (ih->Upflag && !isGrounded && hasDoubleJump) {
    //     //double jump should be able to change velocity direction
    //     velocity.y = .05;
    //     hasDoubleJump = false;
    // }
    
    // //grounded movement
    // if (ih->Leftflag && velocity.x > -MAX_SPEED && isGrounded) {
    //     if (standing) {
    //         velocity.x = -.02;
    //         facingRight = false;
    //     }
    //     else if (!facingRight)
    //         velocity.x -= .002;
    //     standing = false;
    // }
    // if (ih->Rightflag && velocity.x < MAX_SPEED && isGrounded)
    // {
    //     if (standing) {
    //         velocity.x = .02;
    //         facingRight = true;
    //     }
    //     else if (facingRight)
    //         velocity.x += .002;
    //     standing = false;
    // }

    // //arial movment
    // if (ih->Leftflag && (velocity.x > -MAX_AIR_SPEED) && !isGrounded)
    //     velocity.x -= .0006;
    // if (ih->Rightflag && (velocity.x < MAX_AIR_SPEED) && !isGrounded)
    //     velocity.x += .0006;
    
    // //grounded friction
    // if (isGrounded && velocity.x < 0.0f && !ih->Leftflag && !standing)
    // {
    //     if (velocity.x > -.009)
    //         standing = true;
    //     else
    //         velocity.x += .003f;
    // }
    // if (isGrounded && velocity.x > 0.0f && !ih->Rightflag && !standing)
    // {
    //     if (velocity.x < .009)
    //         standing = true;
    //     else
    //         velocity.x -= .003;
    // }

    // if ((velocity.x > -.0005 && velocity.x < .0005) && isGrounded)
    //     standing = true;

    // if (standing) velocity.x = 0.0f;
    
    vec3 u = normalize((camera->lookAtPoint+camera->lookAtOffset) - camera->eye);
    vec3 v = cross(u, camera->up);

    //move Eye + LookAtOffset
    if (ih->Wflag) {
        camera->eye += float(moveVelocity)*vec3(u.x, 0, u.z); 
        camera->lookAtOffset += float(moveVelocity)*vec3(u.x, 0, u.z);
    }
    if (ih->Sflag) {
        camera->eye -= float(moveVelocity)*vec3(u.x, 0, u.z); 
        camera->lookAtOffset -= float(moveVelocity)*vec3(u.x, 0, u.z);
    }
    if (ih->Aflag) {
        camera->eye -= float(moveVelocity)*v; 
        camera->lookAtOffset -= float(moveVelocity)*v;
    }
    if (ih->Dflag) {
        camera->eye += float(moveVelocity)*v; 
        camera->lookAtOffset += float(moveVelocity)*v;
    }
    // if (ih->Shiftflag) {
    //     camera->eye += .5f*float(moveVelocity)*(camera->up); 
    //     camera->lookAtOffset += .5f * float(moveVelocity)*(camera->up);
    // }
    if (ih->Ctrlflag) {
        // camera->eye -= .5f*float(moveVelocity)*(camera->up); 
        // camera->lookAtOffset -= .5f * float(moveVelocity)*(camera->up);
    }
    if (ih->Shiftflag) {moveVelocity = .09;}
    if (!ih->Shiftflag) {moveVelocity = .04;}

    // location = camera->eye;

    // if (standing) cout << "standing ";
    // if (isGrounded) cout << "isGrounded ";
    // cout << location.y << endl;
    camera->eye += velocity;
    return 0;
}