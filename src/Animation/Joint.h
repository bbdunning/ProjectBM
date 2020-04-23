#pragma once
#ifndef JOINT_H
#define JOINT_H
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <assimp/scene.h>


class Joint {
public:
    int index;
    std::string name;
    std::vector<Joint> children;

    //transform from position in model to position in animation (MODEL SPACE)
    //used in shader
    glm::mat4 animatedTransform;

    //transform relative to parent
    glm::mat4 localBindTransform;
    glm::mat4 inverseBindTransform;

    Joint(int index, std::string name, glm::mat4 localBindTransform);
    void calcInverseBindTransform(glm::mat4 *parentBindTransform);
};

class JointTransform
{
public:
    glm::vec3 position;
    glm::quat rotation;

    //CHECK quaternion copying may be wierd, might need to use a pointer
    JointTransform(glm::vec3 position, glm::quat rotation);
    glm::mat4 getLocalTransform();
    JointTransform interpolate(JointTransform frameA, JointTransform frameB, float progression);
};

class KeyFrame
{
public:
    //transform relative to parent joint
    std::map<std::string, JointTransform> pose;
    float timeStamp;

    KeyFrame(float timeStamp, std::map<std::string, JointTransform> pose);
};

#endif