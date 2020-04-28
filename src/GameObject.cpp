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
    // this->setModel(prog);
    for (int i = 0; i < this->shapeList.size(); i++) {
        shapeList[i]->draw(prog);
    }
    this->endTransform.loadIdentity();
}
void GameObject::scale(const glm::vec3 &scaleV) 
{
    this->scaleFactor = scaleV;
    this->endTransform.scale(scaleV);
}
void GameObject::scale(float size) 
{
    this->scaleFactor = this->scaleFactor*size;
    this->endTransform.scale(size);
}
void GameObject::rotate(float angle, const glm::vec3 &axis)
{
    this->endTransform.rotate(angle, axis);
}
void GameObject::translate(const glm::vec3 &offset) 
{
    this->location = offset;
    this->endTransform.translate(offset);
}

void GameObject::setModel(std::shared_ptr<Program> prog) {
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, glm::value_ptr(this->endTransform.topMatrix()));
}
