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

btVector3 bt(vec3 v) {
    return btVector3(v.x, v.y, v.z);
}

vec3 cons(btVector3 v) {
    return vec3(v.getX(), v.getY(), v.getZ());
}

int Player::init(shared_ptr<InputHandler> ih) {
    this->velocity=vec3(0,0,0);
    this->location=vec3(0,0,-2);
    this->ih = ih;
    this->isGrounded = false;
    this->standing = true;
    theta = 0;
    phi = 0;
    prevX = 0;
    prevY = 0;

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
    if (projectileCooldown > 0.0f)
        projectileCooldown -= dt;

    float radius = 50;
    lookAtPoint = vec3(
        radius*cos(phi)*cos(theta),
        radius*sin(phi),
        radius*cos(phi)*cos((PI/2.0)-theta));

    return 0;
}

//get 3d forward direction
vec3 Player::getForwardDir() {
    return normalize(lookAtPoint-location);
}

//get right direction
vec3 Player::getRightDir() {
    return cross(normalize(lookAtPoint-location), vec3(0,1,0));
}

//get normailzed forward direction in 2d plane
vec3 Player::getForwardMoveDir() {
    vec3 forwardDir = this->getForwardDir();
    return normalize(vec3(forwardDir.x, 0, forwardDir.z));
}

float Player::getFacingAngle() {
    return -glm::orientedAngle(normalize(vec3(lookAtPoint.x, 0, lookAtPoint.z)), vec3(1, 0, 0), vec3(0,1,0));
}

// float Player::chargeProjectile() {
// 	if 
// }

void Player::move(float dt, btRigidBody *playerBody, btDynamicsWorld *dynamicsWorld) {
		//move player
		bool playerMoving = false;
		playerBody->forceActivationState(1);
		playerBody->setAngularVelocity(bt(vec3(0,0,0)));
		float forwardSpeed = length(glm::proj(cons(playerBody->getLinearVelocity()), getForwardMoveDir()));
		float strafeSpeed= length(glm::proj(cons(playerBody->getLinearVelocity()), cross(getForwardMoveDir(),vec3(0,1,0))));

		if (ih->Wflag && forwardSpeed < 5) {
			playerMoving = true;
			btVector3 dir = bt(normalize(getForwardMoveDir()));
			float magnitude = 30.f * dt;
			playerBody->applyCentralImpulse(dir * magnitude);
		}
		if (ih->Dflag && strafeSpeed < 5) {
			playerMoving = true;
			btVector3 dir = bt(normalize(getRightDir()));
			float magnitude = 30.f * dt;
			playerBody->applyCentralImpulse(dir * magnitude);
		}
		if (ih->Aflag && strafeSpeed < 5) {
			playerMoving = true;
			btVector3 dir = bt(normalize(-getRightDir()));
			float magnitude = 30.f * dt;
			playerBody->applyCentralImpulse(dir * magnitude);
		}
		if (ih->Sflag && forwardSpeed < 5) {
			playerMoving = true;
			btVector3 dir = bt(normalize(-getForwardMoveDir()));
			float magnitude = 30.f * dt;
			playerBody->applyCentralImpulse(dir * magnitude);
		}
		if (ih->Spaceflag && isGrounded) {
			btVector3 dir = bt(vec3(0,.5,0));
			playerBody->setLinearVelocity(playerBody->getLinearVelocity() + btVector3(0,1.0f,0));
		}

		//raycast straight down
		btVector3 btFrom = bt(this->location);
		btVector3 btTo(this->location.x, this->location.y - .3f, this->location.z);
		btCollisionWorld::ClosestRayResultCallback res(btFrom, btTo);
		dynamicsWorld->rayTest(btFrom, btTo, res);
		if(res.hasHit()){
			isGrounded = true;
		} else {
			isGrounded = false;
		}

		if (playerMoving)
			playerBody->setFriction(1.5f);
		else 
			playerBody->setFriction(3.0f);

		if (isGrounded) {
			playerBody->setGravity(btVector3(0,-10,0));
		}
		else {
			playerBody->setGravity(btVector3(0,-18,0));
		}
}

void Player::updateLocation(btRigidBody *playerBody) {
    btTransform trans;
    vec3 physicsLoc;
    btQuaternion btQ;
    playerBody->getMotionState()->getWorldTransform(trans);
    physicsLoc = vec3(float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
    location = physicsLoc;
}