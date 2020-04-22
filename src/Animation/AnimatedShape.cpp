#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <iostream>
#include <cassert>
#include <assimp/types.h>
#include <memory>
#include <vector>
#include <string>

#include "../GLSL.h"
#include "../Program.h"
#include "../Shape.h" 
#include "AnimatedShape.h"


using namespace std;

// void AnimatedShape::VertexBoneData::AddBoneData(unsigned int BoneID, float Weight)
// {
//     for (unsigned int i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(IDs) ; i++) {
//         if (Weights[i] == 0.0) {
//             IDs[i] = BoneID;
//             Weights[i] = Weight;
//             return;
//         }
//     }

//     // should never get here - more bones than we have space for
//     cout << "BAD" << endl;
//     assert(0);
// } 

// void AnimatedShape::LoadBones(unsigned int MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones)
// {
//     for (unsigned int i = 0 ; i < pMesh->mNumBones ; i++) {
//         unsigned int BoneIndex = 0;
//         string BoneName(pMesh->mBones[i]->mName.data);

//         if (m_BoneMapping->find(BoneName) == m_BoneMapping->end()) {
//             BoneIndex = m_NumBones;
//             m_NumBones++;
//             BoneInfo bi;
//             m_BoneInfo.push_back(bi);
//         }
//         else {
//             BoneIndex = m_BoneMapping[BoneName];
//         }

//         m_BoneMapping[BoneName] = BoneIndex;
//         m_BoneInfo[BoneIndex].BoneOffset = pMesh->mBones[i]->mOffsetMatrix;

//         for (unsigned int j = 0 ; j < pMesh->mBones[i]->mNumWeights ; j++) {
//             unsigned int VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
//             float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
//             Bones[VertexID].AddBoneData(BoneIndex, Weight);
//         }
//     }
// }

void AnimatedShape::createShape(aiMesh* inMesh)
{
	//fil posBuf
	for (int i = 0; i < inMesh->mNumVertices; i++)
		for (int j=0; j<3; j++)
			posBuf.push_back(inMesh->mVertices[i][j]);

	//fill elebuf with triangles
	for (int i = 0; i < inMesh->mNumFaces; i++)
		for (int j=0; j < inMesh->mFaces[i].mNumIndices; j++)
			eleBuf.push_back(inMesh->mFaces[i].mIndices[j]);

	//fill normalBuffer
	for (int i = 0; i < inMesh->mNumVertices; i++) 
		for (int j=0; j < 3; j++)
			norBuf.push_back(inMesh->mNormals[i][j]);

	//fill texBuf
	for (int i=0; i < inMesh->mNumVertices; i++)
		for (int j=0; j < inMesh->mNumUVComponents[0]; j++)
			texBuf.push_back((inMesh->mTextureCoords[0])[i][j]);



	m_BoneMapping = std::make_shared<unordered_map<string, unsigned int>>();

    //fill boneBuf
    Bones.resize(inMesh->mNumVertices);

	for (int i=0; i < inMesh->mNumBones; i++) {
        unsigned int BoneIndex = 0;
        string BoneName = (inMesh->mBones[i]->mName.data);
    }

    // this->LoadBones();

    // m_GlobalInverseTransform = inMesh->mRootNode->mTransformation;
    m_GlobalInverseTransform = glm::mat4(0);
    m_NumBones = 0;

}

void AnimatedShape::init()
{
	texture = nullptr;
	material = nullptr;

	// Initialize the vertex array object - Create the VAO
	CHECKED_GL_CALL(glGenVertexArrays(1, &vaoID));
	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Send the position array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &posBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW));

	// Send the normal array to the GPU
	if (norBuf.empty()) {}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW));
	}

	// Send the texture array to the GPU
	if (texBuf.empty())
	{
		texBufID = 0;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &texBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW));
	}

	// Send the element array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &eleBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW));


    //send JOINT and WEIGHT buf to GPU
    glGenBuffers(1, &boneBufId);
    glBindBuffer(GL_ARRAY_BUFFER, boneBufId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(BONE_ID_LOCATION);
    glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*) 0);
    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

	// Unbind the arrays
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void AnimatedShape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

	if (texture != nullptr)
		(this->texture)->bind(prog->getUniform("Texture0"));

	if (material != nullptr)
		material->setMaterial(prog);

	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if (h_nor != -1 && norBufID != 0)
	{
		GLSL::enableVertexAttribArray(h_nor);
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
	}

	// Bind texcoords buffer
	if (texBufID != 0)
	{
		h_tex = prog->getAttribute("vertTex");

		if (h_tex != -1 && texBufID != 0)
		{
			GLSL::enableVertexAttribArray(h_tex);
			CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
			CHECKED_GL_CALL(glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
		}
	}

	// Bind element buffer
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));

	// Draw
	CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0));

	// Disable and unbind
	if (h_tex != -1)
	{
		GLSL::disableVertexAttribArray(h_tex);
	}
	if (h_nor != -1)
	{
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
