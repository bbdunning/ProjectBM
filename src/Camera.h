#pragma once
#ifndef CAMERA_H
#define CAMERA_H
#include <memory>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 eye;
    glm::vec3 lookAtPoint;
    glm::vec3 lookAtOffset;
    glm::vec3 up;

    Camera();
    void init();
};


#endif