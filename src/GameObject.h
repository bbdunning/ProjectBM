#pragma once
#ifndef _MESH_H_
#define _MESH_H_

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
        
        void setModel(std::shared_ptr<Program> prog);
        void draw(std::shared_ptr<Program> prog);
        void init();
	    // void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M);
        void translate(const glm::vec3 &offset);
        void scale(const glm::vec3 &scaleV);
        void scale(float size);
        void rotate(float angle, const glm::vec3 &axis);
        // static void createGameObject();
    
    // protected:
        MatrixStack endTransform;

};

#endif