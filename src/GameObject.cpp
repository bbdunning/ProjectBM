#include <memory>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include "Shape.h"
#include "GameObject.h"
#include "Program.h"

void GameObject::init() {
    name = nullptr;
    location = glm::vec3(0,0,0);
}

void GameObject::draw(std::shared_ptr<Program> prog) {
    glm::mat4 Model;
    for (int i = 0; i < this->shapeList.size(); i++) {
        shapeList[i]->draw(prog);
    }
}