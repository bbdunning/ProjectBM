#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cassert>
#include <assimp/types.h>
#include <glm/gtx/string_cast.hpp>
#include <memory>
#include <vector>
#include <string>

#include "../GLSL.h"
#include "../Program.h"
#include "../Shape.h" 
#include "AnimatedShape.h"
#define MAX_JOINTS 4


using namespace std;
using namespace glm;

void AnimatedShape::update() {
	this->animator.update();
	this->setJointTransforms();
}

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

	jointIdBuf.resize(inMesh->mNumVertices * MAX_JOINTS);
	jointWeightBuf.resize(inMesh->mNumVertices * MAX_JOINTS);

	map<int, int> countmap;
	for (int i=0; i<inMesh->mNumVertices;i++) {
		countmap[i]=0;
	}

	for (int i=0; i<inMesh->mNumBones; i++) {
		cout << inMesh->mBones[i]->mName.C_Str() << " " << inMesh->mBones[i]->mNumWeights << endl;
		for (int j=0; j<inMesh->mBones[i]->mNumWeights; j++) {
			int vertexId = inMesh->mBones[i]->mWeights[j].mVertexId;
			float vertexWeight = inMesh->mBones[i]->mWeights[j].mWeight;
			// cout << vertexId << " " << vertexWeight << endl;
			//set index
			if (countmap[vertexId] < 4) {
				string name = inMesh->mBones[i]->mName.C_Str(); Joint q = (*joints)[(*jointMap)[name]];
				jointIdBuf[(vertexId*MAX_JOINTS) + countmap[vertexId]] = q.index;
				//set weight
				jointWeightBuf[(vertexId*MAX_JOINTS) + countmap[vertexId]] = vertexWeight;
				countmap[vertexId]++;
			} else {
				cout << " REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE " << endl;
			}
		}
	}
}

//should make sure these are working correctly; make sure vector is changing
//gets an array of joint model transforms
vector<mat4> AnimatedShape::getJointTransforms() {
	vector<mat4> jointMatrices;
	jointMatrices.resize(jointCount);
	addJointsToArray(rootJoint, jointMatrices);
	return jointMatrices;
}

//adds current model-space transform (to parents?)
void AnimatedShape::addJointsToArray(Joint *headJoint, vector<mat4> jointMatrices) {
	jointMatrices[headJoint->index] = headJoint->animatedTransform;
	for (int i=0; i<headJoint->children.size(); i++) {
		addJointsToArray(headJoint->children[i], jointMatrices);
	}
}

void AnimatedShape::init(Joint *rootJoint)
{
	this->rootJoint = rootJoint;
	if (rootJoint != nullptr)
		rootJoint->calcInverseBindTransform(new mat4(1));
	if (this->isAnimated) {
		this->animator = Animator();
		animator.entity = this;
	}
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



    CHECKED_GL_CALL(glGenBuffers(1, &jointBufId));
    CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, jointBufId));
    CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(sizeof(float)) * jointIdBuf.size(), &jointIdBuf[0], GL_STATIC_DRAW));

    CHECKED_GL_CALL(glGenBuffers(1, &weightBufId));
    CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, weightBufId));
    CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(sizeof(unsigned int)) * jointWeightBuf.size(), &jointWeightBuf[0], GL_STATIC_DRAW));

	// Unbind the arrays
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void AnimatedShape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex, h_jid, h_wid;
	h_pos = h_nor = h_tex = h_jid = h_wid = -1;

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

	//Bind jointBuf
	if (isAnimated) {
		if (jointBufId != 0) {
			h_jid = prog->getAttribute("jointIndices");

			if (h_jid != -1 && jointBufId != 0) {
				CHECKED_GL_CALL(glEnableVertexAttribArray(BONE_ID_LOCATION));
				CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, jointBufId));
				CHECKED_GL_CALL(glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, 0, (const GLvoid*) 0));
			}
		}

		if (weightBufId != 0) {
			h_wid = prog->getAttribute("jointWeights");

			if (h_wid != -1 && weightBufId != 0) {
				CHECKED_GL_CALL(glEnableVertexAttribArray(BONE_WEIGHT_LOCATION));
				CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, weightBufId));
				CHECKED_GL_CALL(glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0));
			}
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

// inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
// {
//     glm::mat4 to;


//     to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
//     to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
//     to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
//     to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;

//     return to;
// }

void AnimatedShape::setJointTransforms() {
	for (int i=0; i<this->joints->size(); i++) {
		this->jointTransforms[(*joints)[i].index] = (*joints)[i].animatedTransform;
		// this->jointTransforms[(*joints)[i].index] = mat4(1);
	}
}