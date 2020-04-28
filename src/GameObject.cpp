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
    location = glm::vec3(0,0,0);
}

shared_ptr<GameObject> GameObject::create(string meshPath, string fileName, string objName) {
    Assimp::Importer importer;
    shared_ptr<GameObject> mesh = make_shared<GameObject>();
    mesh->name = objName;
    Joint *rootJoint = nullptr;
    vector<Animation> animList; 
    shared_ptr<vector<Joint>> joints = make_shared<vector<Joint>>();
    shared_ptr<map<string, unsigned int>> jointMap = make_shared<map<string, unsigned int>>();

    const aiScene* scene = importer.ReadFile(
        meshPath + fileName, aiProcess_Triangulate | aiProcess_FlipUVs);

    cout << "creating " << objName << endl;
    populateJointMap(jointMap, scene->mRootNode, scene, joints);
    // for (map<string, unsigned int>::iterator it = jointMap->begin(); it != jointMap->end(); ++it)
    // 	cout << it->first << " " << it->second->name << endl;

    buildJointHeirarchy(jointMap, joints, scene->mRootNode, scene);
    createAnimations(scene, animList);
    // fillAnimations(animList, *joints);
    // printAnimations(animList);
    // printAllJoints(jointMap);
    // for (int i=0; i<joints->size(); i++) {
    // 	cout<< "joint: " << (*joints)[i].name << " has children: " << (*joints)[i].children.size() << endl;
    // }


    if (joints->size() > 0) {
        rootJoint = getRootJoint(jointMap, joints, scene->mRootNode);
        cout << "root joint name: " << rootJoint->name << endl;
        // printJoints(rootJoint);
        mat4 temp(1.0f);
    }

    for (int i=0; i< scene->mNumMeshes; i++) {
        mesh->shapeList.push_back(createShape(scene, meshPath, fileName, objName, mesh, i, rootJoint, jointMap, joints));
    }
    if (animList.size() > 0) {
    shared_ptr<Animation> animation = make_shared<Animation>(animList[0].length, animList[0].frames);
        for (int i=0; i<mesh->shapeList.size(); i++) {
            mesh->shapeList[i]->animator.doAnimation(animation);
        }
    }

    return mesh;
    // objL[objName] = mesh;
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
