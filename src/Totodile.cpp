#include <iostream>
#include <cmath>
#include <unordered_map>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "GameObject.h"
#include "Texture.h"
#include "stb_image.h"
#include "InputHandler.h"
#include "Player.h"
#include "Animation/AnimatedShape.h"
#include "Camera.h"
#include "Totodile.h"

using namespace std;
using namespace glm;

void Totodile::init(std::vector<std::shared_ptr<Shape>> totoMesh) {
    typedef GameObject super;
    super::init();
    this->shapeList = totoMesh;
    lookAtPoint = location + vec3(rand()%10-5, 0, rand()%10-5);
    vec3 velocityDir = normalize(lookAtPoint-location);
    velocity = velocityDir * .05f;
    randSeed = rand();
}

void Totodile::draw(shared_ptr<Program> prog) {
    auto Model = make_shared<MatrixStack>();
    Model->translate(this->location);
    Model->scale(vec3(.025,.025, .025));

    vec3 d = normalize(vec3(velocity.x, 0, velocity.z));
    float omega = glm::acos(glm::dot(vec3(0,0,1), d));
    if (velocity.x < 0) {
        omega = -omega;
    }
    cout << "omega " << omega << endl;

    // vec3 a = glm::cross(vec3(0,1,0));
    Model->rotate(.5*sin(glfwGetTime()*12+randSeed), d);
    Model->rotate(omega, vec3(0, 1, 0));
    // Model->rotate(90, vec3(0, 1, 0));


    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
    for (int i = 0; i < this->shapeList.size(); i++) {
        shapeList[i]->draw(prog);
    }
}

void Totodile::update() {
    this->location += velocity;
    this->lookAtPoint += velocity;
}

// Totodile Totodile::spawn() {

// }