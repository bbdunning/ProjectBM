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

void GameObject::init() {
    this->name = "";
    this->location = glm::vec3(0,0,0);
}

void GameObject::draw(std::shared_ptr<Program> prog) {
    for (int i = 0; i < this->shapeList.size(); i++) {
        shapeList[i]->draw(prog);
    }
}