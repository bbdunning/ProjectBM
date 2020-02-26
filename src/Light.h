#pragma once
#ifndef _LIGHT_H_
#define _LIGHT_H_

#include <memory>
#include "Shape.h"
#include <glm/gtc/matrix_transform.hpp>


class Light {
    public:
        glm::vec3 color;
};

class PointLight: public Light {
    public:
        glm::vec3 position;
        glm::vec3 intensity;
};

class DirectedLight: public Light {
    public:
        glm::vec3 direction;
        glm::vec3 intensity;
};

#endif