#pragma once
#ifndef _MESH_H_
#define _MESH_H_

#include <memory>
#include "Shape.h"
#include "Animation/AnimatedShape.h"
#include <glm/gtc/type_ptr.hpp>


class GameObject {
    public:
        std::string name;
        std::vector<std::shared_ptr<AnimatedShape>> shapeList;
        glm::vec3 location;
        
        void draw(std::shared_ptr<Program> prog);
        void init();
        // static void createGameObject();
};

#endif