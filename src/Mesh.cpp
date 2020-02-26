#include <memory>
#include <glm/gtc/type_ptr.hpp>

#include "Shape.h"
#include "Mesh.h"
#include "Program.h"

void Mesh::draw(std::shared_ptr<Program> prog) {
    for (int i = 0; i < this->shapeList.size(); i++) {
        shapeList[i]->draw(prog);
    }
}