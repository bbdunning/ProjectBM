#pragma once
#ifndef JOINT_H
#define JOINT_H
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <unordered_map>
#include <iostream>
#include <cassert>
#include <assimp/types.h>
#include <assimp/quaternion.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/texture.h>
#include <assimp/postprocess.h>
#include <memory>
#include <vector>
#include <string>

#include "../GLSL.h"
#include "../Program.h"
#include "../Shape.h" 

static inline glm::vec3 vec3_cast(const aiVector3D &v) { return glm::vec3(v.x, v.y, v.z); }
static inline glm::quat quat_cast(const aiQuaternion &q) { return glm::quat(q.w, q.x, q.y, q.z); }
static inline glm::mat4 mat4_cast(const aiMatrix4x4 &m) { return glm::transpose(glm::make_mat4(&m.a1)); }
static inline glm::mat4 mat4_cast(const aiMatrix3x3 &m) { return glm::transpose(glm::make_mat3(&m.a1)); }


class Joint {
public:
    int index;
    std::string name;
    std::vector<Joint*> children;

    //transform from position in model to position in animation (MODEL SPACE)
    //used in shader
    glm::mat4 animatedTransform;

    //transform relative to parent
    glm::mat4 localBindTransform;
    glm::mat4 inverseBindTransform;

    Joint() {};
    Joint(int index, std::string name, glm::mat4 localBindTransform);
    void calcInverseBindTransform(glm::mat4 *parentBindTransform);
};

void buildJointHeirarchy(std::shared_ptr<std::map<std::string, unsigned int>> jointMap, std::shared_ptr<std::vector<Joint>> &joints, aiNode *node, const aiScene* scene);
Joint* getRootJoint(std::shared_ptr<std::map<std::string, unsigned int>> jointMap, std::shared_ptr<std::vector<Joint>> &joints, aiNode *node);
void printJoints(Joint *j);
void printAllJoints(std::vector<Joint> joints);
void printTransforms(std::vector<glm::mat4> t);

class JointTransform
{
public:
    glm::vec3 position;
    glm::quat rotation;

    //CHECK quaternion copying may be wierd, might need to use a pointer
    JointTransform() {};
    JointTransform(glm::vec3 position, glm::quat rotation);
    glm::mat4 getLocalTransform();
    static JointTransform interpolate(JointTransform frameA, JointTransform frameB, float progression);
};

class KeyFrame
{
public:
    //transform relative to parent joint
    std::map<std::string, JointTransform> pose;
    float timeStamp;

    KeyFrame(float timeStamp, std::map<std::string, JointTransform> pose);
    std::vector<std::string> getJointKeyFrames();
};

#endif