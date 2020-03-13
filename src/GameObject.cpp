#include <memory>
#include <glm/gtc/type_ptr.hpp>

#include "Shape.h"
#include "GameObject.h"
#include "Program.h"

void GameObject::draw(std::shared_ptr<Program> prog) {
    for (int i = 0; i < this->shapeList.size(); i++) {
        shapeList[i]->draw(prog);
    }
}