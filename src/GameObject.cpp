#include <memory>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include "Shape.h"
#include "GameObject.h"
#include "Program.h"

using namespace std;
using namespace glm;

void GameObject::init() {
    name = nullptr;
    path = nullptr;
    location = glm::vec3(0,0,0);
}

shared_ptr<GameObject> GameObject::create(string meshPath, string fileName, string objName) {
    Assimp::Importer importer;
    shared_ptr<GameObject> mesh = make_shared<GameObject>();
    Joint *rootJoint = nullptr;
    vector<shared_ptr<Animation>> animList; 
    shared_ptr<vector<Joint>> joints = make_shared<vector<Joint>>();
    shared_ptr<map<string, unsigned int>> jointMap = make_shared<map<string, unsigned int>>();

    mesh->name = objName;
    mesh->path = meshPath;

    const aiScene* scene = importer.ReadFile(
        meshPath + fileName, aiProcess_Triangulate | aiProcess_FlipUVs);

    cout << "creating " << objName << endl;
    populateJointMap(jointMap, scene->mRootNode, scene, joints);
    buildJointHeirarchy(jointMap, joints, scene->mRootNode, scene);
    createAnimations(scene, animList);
    cout << "animations: " << endl;
    printAnimations(animList);
    mesh->animList = animList;
    // printAllJoints(jointMap);
    // for (int i=0; i<joints->size(); i++) {
    // 	cout<< "joint: " << (*joints)[i].name << " has children: " << (*joints)[i].children.size() << endl;
    // }


    if (joints->size() > 0) {
        rootJoint = getRootJoint(jointMap, joints, scene->mRootNode);
        cout << "\nroot joint name: " << rootJoint->name << endl;
        // printJoints(rootJoint);
        mat4 temp(1.0f);
    }

    for (int i=0; i< scene->mNumMeshes; i++) {
        mesh->shapeList.push_back(createShape(scene, meshPath, fileName, objName, mesh, i, rootJoint, jointMap, joints));
    }
    mesh->doAnimation(0);
    return mesh;
}

void GameObject::doAnimation(int animNum) {
    if (animNum == currentAnimationNum)
        return;
    if (animList.size() > animNum) {
        for (int i=0; i<shapeList.size(); i++) {
            shapeList[i]->animator.doAnimation(animList[animNum]);
        }
    } else {
        cerr << "BAD ANIMATION NUMBER" << endl;
        return;
    }
    currentAnimationNum = animNum;
}

void GameObject::addAnimation(string fileName) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        this->path + fileName, aiProcess_Triangulate | aiProcess_FlipUVs);
    createAnimations(scene, animList);
}


void GameObject::draw(std::shared_ptr<Program> prog) {
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
    location = offset;
    this->endTransform.translate(offset);
}

void GameObject::setModel(std::shared_ptr<Program> prog) {
    glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, glm::value_ptr(this->endTransform.topMatrix()));
}
