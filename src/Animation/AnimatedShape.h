#pragma once
#ifndef ANIMATEDSHAPE_H
#define ANIMATEDSHAPE_H
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <memory>
#include <assimp/scene.h>

#include "Joint.h"
#include "Animator.h"
#include "../Shape.h"

#define NUM_BONES_PER_VERTEX 4
#define BONE_ID_LOCATION 3
#define BONE_WEIGHT_LOCATION 4

class GameObject;

class AnimatedShape: public Shape
{
public:
    std::string name;
    const aiScene* scene;
    Joint *rootJoint;
    int jointCount;
    Animator animator;
    bool isAnimated;
    std::vector<int> jointIdBuf;
    std::vector<float> jointWeightBuf;
    std::shared_ptr<std::map<std::string, unsigned int>> jointMap;
    std::shared_ptr<std::vector<Joint>> joints;
    std::vector<glm::mat4> jointTransforms;
    unsigned int jointBufId = 0;
    unsigned int weightBufId = 0;


    void init(Joint* rootJoint);
	void createShape(aiMesh* inMesh) override;
	void draw(const std::shared_ptr<Program> prog) const override;
	std::vector<glm::mat4> getJointTransforms();
    void addJointsToArray(Joint *headJoint, std::vector<glm::mat4> jointMatrices);
    void setJointTransforms();
    void update(); //updates animator
    
private:
};

std::shared_ptr<AnimatedShape> createShape(const aiScene * scene, std::string meshPath, 
	std::string fileName, std::string objName, std::shared_ptr<GameObject> obj, int i, Joint *rootJoint, 
	std::shared_ptr<std::map<std::string, unsigned int>> jointMap, std::shared_ptr<std::vector<Joint>> joints);

#endif