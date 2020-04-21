#pragma once
#ifndef ANIMATEDSHAPE_H
#define ANIMATEDSHAPE_H
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <string>
#include <memory>

#include "../Shape.h"

#define NUM_BONES_PER_VERTEX 4
#define BONE_ID_LOCATION 3
#define BONE_WEIGHT_LOCATION 4

struct VertexBoneData
{
    unsigned int IDs[NUM_BONES_PER_VERTEX];
    float weights[NUM_BONES_PER_VERTEX];
};

class AnimatedShape: public Shape
{
public:
    std::vector<VertexBoneData> Bones;
    unsigned int boneBufId;
    glm::mat4 m_GlobalInverseTransform;
	std::shared_ptr<std::unordered_map<std::string, unsigned int>> m_BoneMapping;
    unsigned int m_NumBones;

    void init() override;
	void createShapeFromAssimp(aiMesh* inMesh) override;
	void draw(const std::shared_ptr<Program> prog) const override;
    void LoadBones(unsigned int MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones);
    
    // std::vector<int> jointIdBuf;
    // std::vector<float> vertexWeightBuf;

    // unsigned int jointIdBufID;
    // unsigned int vertexWeightBufID;
};

#endif