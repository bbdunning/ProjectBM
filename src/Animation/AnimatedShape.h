#pragma once
#ifndef ANIMATEDSHAPE_H
#define ANIMATEDSHAPE_H
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <assimp/scene.h>

#include "Joint.h"
#include "Animator.h"
#include "../Shape.h"

#define NUM_BONES_PER_VERTEX 4
#define BONE_ID_LOCATION 3
#define BONE_WEIGHT_LOCATION 4


class AnimatedShape: public Shape
{
public:
    const aiScene* scene;
    Joint *rootJoint;
    int jointCount;
    Animator animator;
    bool isAnimated;


    void init(Joint* rootJoint);
	void createShape(aiMesh* inMesh) override;
	void draw(const std::shared_ptr<Program> prog) const override;
	std::vector<glm::mat4> getJointTransforms();
    void addJointsToArray(Joint *headJoint, std::vector<glm::mat4> jointMatrices);
    //updates animator
    void update();
    
    // std::vector<int> jointIdBuf;
    // std::vector<float> vertexWeightBuf;

    // unsigned int jointIdBufID;
    // unsigned int vertexWeightBufID;
private:
};

#endif