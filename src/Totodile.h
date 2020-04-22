#pragma once
#ifndef _TOTODILE_H_
#define _TOTODILE_H_

#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shape.h"
#include "GameObject.h"


class Totodile : public GameObject {
    public:
        glm::vec3 lookAtPoint;
        glm::vec3 velocity;
        int randSeed;
        float scale;
        bool isCaught;
        void init(std::vector<std::shared_ptr<Shape>>);
        void draw(std::shared_ptr<Program> prog);
        void update();
};

#endif