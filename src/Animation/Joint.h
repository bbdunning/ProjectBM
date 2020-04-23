#pragma once
#ifndef JOINT_H
#define JOINT_H
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <iostream>
#include <cassert>
#include <assimp/types.h>
#include <assimp/quaternion.h>
#include <memory>
#include <vector>
#include <string>

#include "../GLSL.h"
#include "../Program.h"
#include "../Shape.h" 

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
    std::shared_ptr<glm::mat4> localBindTransform;
    glm::mat4 inverseBindTransform;

    Joint() {};
    Joint(int index, std::string name, std::shared_ptr<glm::mat4> localBindTransform);
    void calcInverseBindTransform(glm::mat4 *parentBindTransform);
};

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