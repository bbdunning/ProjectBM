#pragma once
#ifndef CAMERA_H
#define CAMERA_H
#include <memory>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "InputHandler.h"

class Camera {
public:
    glm::vec3 eye;
    glm::vec3 lookAtPoint;
    glm::vec3 up;
	glm::vec3 elevation;
	float phi;
	float distance;
	float theta;
	float prevX;
	float prevY;
    std::shared_ptr<InputHandler> inputHandler;
    float moveVelocity;

    void init();
    glm::mat4 getViewMatrix();
    void setInputHandler(std::shared_ptr<InputHandler> ih);
    void setViewAngles(GLFWwindow *window);
	bool checkInFrustum(const glm::mat4 m, glm::vec4 v);
};


#endif