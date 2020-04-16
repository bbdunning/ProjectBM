#include <memory>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include "Shape.h"
#include "GameObject.h"
#include "Program.h"

void GameObject::draw(std::shared_ptr<Program> prog) {
    std::cout << "drawing: " << this->name << std::endl;
    for (int i = 0; i < this->shapeList.size(); i++) {
        shapeList[i]->draw(prog);
    }
}