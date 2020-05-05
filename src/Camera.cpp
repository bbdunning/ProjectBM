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
	up = vec3(0,1,0);
}

void Camera::setInputHandler(shared_ptr<InputHandler> ih) {
    inputHandler = ih;
}

mat4 Camera::getViewMatrix() {
    int radius = 50;

    // lookAtPoint = vec3(
    //     radius*cos(phi)*cos(theta),
    //     radius*sin(phi),
    //     radius*cos(phi)*cos((PI/2.0)-theta));

    vec3 u = normalize(lookAtPoint);
    vec3 v = cross(u, up);

    //move Eye + LookAtOffset
    // if (inputHandler->Wflag) {eye += float(moveVelocity)*u;}
    // if (inputHandler->Sflag) {eye -= float(moveVelocity)*u;}
    // if (inputHandler->Aflag) {eye -= float(moveVelocity)*v;}
    // if (inputHandler->Dflag) {eye += float(moveVelocity)*v;}
    // if (inputHandler->Shiftflag) {eye += .5f*float(moveVelocity)*(up);}
    // if (inputHandler->Ctrlflag) {eye -= .5f*float(moveVelocity)*(up);}
    // if (inputHandler->Shiftflag) {moveVelocity = .09;}
    // if (!inputHandler->Shiftflag) {moveVelocity = .04;}

    return glm::lookAt(eye, lookAtPoint + eye, up);
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

bool Camera::checkInFrustum(const glm::mat4 m, glm::vec4 v) {
    vec4 v2 = m*v; 
    bool l, r, b, t, n = 1, f = 1;

    //check against left plane
    float A = m[0][3] + m[0][0];
    float B = m[1][3] + m[1][0];
    float C = m[2][3] + m[2][0];
    float D = m[3][3] + m[3][0];
    l = (0 < A*v.x + B*v.y + C*v.z + D);

    //check against right plane
    A = m[0][3] - m[0][0];
    B = m[1][3] - m[1][0];
    C = m[2][3] - m[2][0];
    D = m[3][3] - m[3][0];
    r = (0 < A*v.x + B*v.y + C*v.z + D);

    //bottom
    A = m[0][3] + m[0][1];
    B = m[1][3] + m[1][1];
    C = m[2][3] + m[2][1];
    D = m[3][3] + m[3][1];
    b = (0 < A*v.x + B*v.y + C*v.z + D);

    //top
    A = m[0][3] - m[0][1];
    B = m[1][3] - m[1][1];
    C = m[2][3] - m[2][1];
    D = m[3][3] - m[3][1];
    t = (0 < A*v.x + B*v.y + C*v.z + D);

    //near
    A = m[0][3] + m[0][2];
    B = m[1][3] + m[1][2];
    C = m[2][3] + m[2][2];
    D = m[3][3] + m[3][2];
    n = (0 < A*v.x + B*v.y + C*v.z + D);

    //far
    A = m[0][3] - m[0][2];
    B = m[1][3] - m[1][2];
    C = m[2][3] - m[2][2];
    D = m[3][3] - m[3][2];
    f = (0 < A*v.x + B*v.y + C*v.z + D);

    return l && r && b && t && n && f;
}