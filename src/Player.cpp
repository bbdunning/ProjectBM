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
#define PI 3.14159
#define viewFactor .002

Player::Player() {
}

int Player::init(shared_ptr<InputHandler> ih) {
    this->velocity=vec3(0,0,0);
    // this->location=vec3(0,-.7,-2);
    this->location=vec3(0,0,-2);
    this->ih = ih;
    this->isGrounded = true;
    this->standing = true;
    this->hasDoubleJump = true;
    this->facingRight = true;
    theta = 0;
    phi = 0;
    prevX = 0;
    prevY = 0;

    this->environmentalHbox = HitSphere(this->location, .008);
    return 0;
}

void Player::setViewAngles(GLFWwindow *window) {
    double posX, posY;
    glfwGetCursorPos(window, &posX, &posY);

    theta += viewFactor *(posX - this->prevX);
    phi -= (viewFactor * (posY - this->prevY));
    phi = fmax(phi, -PI/2 + 0.2);
    phi = fmin(phi, PI/2 - 0.2);

    this->prevX = posX;
    this->prevY = posY;
}

int Player::update(float dt) {
    // int axesCount;
    // const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
    // std::cout << "Left Stick X Axis: " << axes[0] << std::endl;
    this->environmentalHbox.center = this->location;
    bool isOnPlatform = false;

    vec3 forward = normalize(vec3(lookAtPoint.x - location.x, 0, lookAtPoint.z - location.z));
    vec3 right = cross(normalize(lookAtPoint-location), vec3(0,1,0));

    float radius = 50;
    lookAtPoint = vec3(
        radius*cos(phi)*cos(theta),
        radius*sin(phi),
        radius*cos(phi)*cos((PI/2.0)-theta));


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
    for (int i=1; i<cd->environmentBoxes.size(); i++) {
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
    if (cd->check(*cd->environmentBoxes[0], this->environmentalHbox) && velocity.y <=0) {// && !isGrounded) {
        if (!isGrounded) {
            standing=true;
        }
        isGrounded = true;
        velocity.y = 0;
        hasDoubleJump = true;
        isOnPlatform = true;
        this->location.y = cd->environmentBoxes[0]->max.y;
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
        velocity.y = .2; //shorthop
        velocity.x = clamp(velocity.x, -0.018f, 0.018f);
        velocity.z = clamp(velocity.z, -0.018f, 0.018f);
        isGrounded = false;
        standing = false;
    }
    

    if (standing) {
        velocity.x = 0.0f;
        velocity.z = 0.0f;
    }

    if (ih->R) {
        this->isAttacking = true;
    } else {
        this->isAttacking = false;
    }
    
    vec3 velocityDir = vec3(0,0,0);
    if (ih->Wflag)
        velocity += forward;
    if (ih->Sflag)
        velocity -= forward;
    if (ih->Aflag)
        velocity -= right;
    if (ih->Dflag)
        velocity += right;

    if (velocity != vec3(0))
        velocityDir = normalize(velocity);

    float speed =.04;
    if (ih->Shiftflag)
        speed = .08;

    location += velocityDir * speed;
    location += vec3(0, velocity.y, 0);
    cout << location.x << " " << location.y << " " << location.z << endl;
    return 0;
}

vec3 Player::getForwardDir() {
    return normalize(lookAtPoint-location);
}

vec3 Player::getRightDir() {
    return cross(normalize(lookAtPoint-location), vec3(0,1,0));
}


Sandbag::Sandbag() {
}

int Sandbag::init(shared_ptr<InputHandler> ih) {
    this->velocity=vec3(0,0,0);
    // this->location=vec3(0,-.7,-2);
    this->location=vec3(1,-1,-2);
    this->ih = ih;
    this->isGrounded = true;
    this->standing = true;
    this->hasDoubleJump = true;
    this->facingRight = true;

    this->environmentalHbox = HitSphere(this->location, .008);
    return 0;
}

int Sandbag::checkCollisions(vector<HitSphere> &hitboxes){
    for (int i=0; i<hitboxes.size(); i++) {
        if (CollisionDetector::sphereCheck(this->environmentalHbox, hitboxes[i])) {
            if (this->environmentalHbox.center.x < hitboxes[i].center.x) {
                return -1;
            } else {
                return 1;
            }
        }
    }
    return 0;
}

int Sandbag::update(vector<HitSphere> &hitboxes) {
    // int axesCount;
    // const float *axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
    // std::cout << "Left Stick X Axis: " << axes[0] << std::endl;
    this->environmentalHbox.center = this->location;
    bool isOnPlatform = false;

    if (this->checkCollisions(hitboxes) != 0) {
        cout << this->checkCollisions(hitboxes) << endl;
        this->currentPercent += .06;
        if (this->checkCollisions(hitboxes) == 1) {
            this->velocity = vec3(.2,.2,0) * currentPercent;
        } else {
            this->velocity = vec3(-.2,.2,0) * currentPercent;
        }
        isGrounded = false;
        standing = false;
    }
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
    for (int i=1; i<cd->environmentBoxes.size(); i++) {
        if (cd->check(*cd->environmentBoxes[i], this->environmentalHbox) && velocity.y <= 0 && !ih->kp2) {// && !isGrounded) {
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
    if (cd->check2(*cd->environmentBoxes[0], this->environmentalHbox) && velocity.y <=0) {// && !isGrounded) {
        if (!isGrounded) {
            standing=true;
        }
        isGrounded = true;
        velocity.y = 0;
        hasDoubleJump = true;
        isOnPlatform = true;
        this->location.y = cd->environmentBoxes[0]->max.y;
    } 
    if (!isOnPlatform || ih->kp2) {
        isGrounded = false;
    }


    //gravity
    if (!isGrounded && velocity.y < MAX_GRAVITY)
        velocity.y -= .004;

    //jump
    if (ih->kp5 && isGrounded) {
        // velocity.y += .065; //fullhop
        velocity.y = .07; //shorthop
        velocity.x = clamp(velocity.x, -0.018f, 0.018f);
        isGrounded = false;
        standing = false;
    }
    
    //double jump
    // if (ih->Upflag && !isGrounded && hasDoubleJump) {
        //double jump should be able to change velocity direction
        // velocity.y = .05;
        // hasDoubleJump = false;
    // }
    
    //grounded movement
    if (ih->kp1 && velocity.x > -MAX_SPEED && isGrounded) {
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
    if (ih->kp3 && velocity.x < MAX_SPEED && isGrounded)
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
    if (ih->kp1 && (velocity.x > -MAX_AIR_SPEED) && !isGrounded)
        velocity.x -= .0009;
    if (ih->kp3 && (velocity.x < MAX_AIR_SPEED) && !isGrounded)
        velocity.x += .0009;
    
    //grounded friction
    if (isGrounded && velocity.x < 0.0f && !ih->kp1 && !standing)
    {
        //stop and stand
        if (velocity.x > -.009)
            standing = true;
        //slow down
        else
            velocity.x += .002f;
    }
    if (isGrounded && velocity.x > 0.0f && !ih->kp3 && !standing)
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