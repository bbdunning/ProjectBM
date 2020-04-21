#include <memory>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "WindowManager.h"
#include "InputHandler.h"
#include "Camera.h"
#define viewFactor .002
#define PI 3.14159

using namespace std;
using namespace glm;

void Camera::init() {
	phi = 0;
	theta = PI;
	prevX = 0;
	prevY = 0;
    moveVelocity = .04;

	eye = vec3(0,0,5);
	lookAtPoint = vec3(
		cos(phi)*cos(theta),
		sin(phi),
		cos(phi)*cos((PI/2.0)-theta));
	lookAtOffset = vec3(0,0,0);
	up = vec3(0,1,0);
}

void Camera::setInputHandler(shared_ptr<InputHandler> ih) {
    inputHandler = ih;
}

mat4 Camera::getViewMatrix() {
    int radius = 50;
    int step = .5;

    lookAtPoint = vec3(
        radius*cos(phi)*cos(theta),
        radius*sin(phi),
        radius*cos(phi)*cos((PI/2.0)-theta));

    vec3 u = normalize((lookAtPoint+lookAtOffset) - eye);
    vec3 v = cross(u, up);

    //move Eye + LookAtOffset
    if (inputHandler->Wflag) {eye += float(moveVelocity)*u; lookAtOffset += float(moveVelocity)*u;}
    if (inputHandler->Sflag) {eye -= float(moveVelocity)*u; lookAtOffset -= float(moveVelocity)*u;}
    if (inputHandler->Aflag) {eye -= float(moveVelocity)*v; lookAtOffset -= float(moveVelocity)*v;}
    if (inputHandler->Dflag) {eye += float(moveVelocity)*v; lookAtOffset += float(moveVelocity)*v;}
    if (inputHandler->Shiftflag) {eye += .5f*float(moveVelocity)*(up); lookAtOffset += .5f * float(moveVelocity)*(up);}
    if (inputHandler->Ctrlflag) {eye -= .5f*float(moveVelocity)*(up); lookAtOffset -= .5f * float(moveVelocity)*(up);}
    if (inputHandler->Shiftflag) {moveVelocity = .09;}
    if (!inputHandler->Shiftflag) {moveVelocity = .04;}

    return glm::lookAt(eye, lookAtPoint + lookAtOffset, up);
}

void Camera::setViewAngles(GLFWwindow *window) {
    double posX, posY;
    glfwGetCursorPos(window, &posX, &posY);

    theta += viewFactor *(posX - this->prevX);
    phi -= (viewFactor * (posY - this->prevY));
    phi = fmax(phi, -PI/2 + 0.2);
    phi = fmin(phi, PI/2 - 0.2);

    this->prevX = posX;
    this->prevY = posY;
}