#pragma once
#ifndef _GAMEOBJECT_H_
#define _GAMEOBJECT_H_

#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "Shape.h"
#include "Animation/AnimatedShape.h"
#include "MatrixStack.h"


class GameObject {
    public:
        std::string name;
        std::vector<std::shared_ptr<AnimatedShape>> shapeList;
        glm::vec3 location = glm::vec3(0,0,0);
        glm::vec3 scaleFactor = glm::vec3(1,1,1);
        glm::vec3 min, max;
        
        void init();
        void draw(std::shared_ptr<Program> prog);
        void setModel(std::shared_ptr<Program> prog);
        // void measure();
        void translate(const glm::vec3 &offset);
        void scale(const glm::vec3 &scaleV);
        void scale(float size);
        void rotate(float angle, const glm::vec3 &axis);
    
    protected:
        MatrixStack endTransform;

};

#endif