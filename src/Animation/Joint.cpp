#include "Joint.h"
#include "Animator.h"
#include <glm/gtx/quaternion.hpp>

using namespace glm;
using namespace std;

Joint::Joint(int index, std::string name, shared_ptr<glm::mat4> localBindTransform)
{
    this->index = index;
    this->name = name;
    this->localBindTransform = localBindTransform;
    animatedTransform;
}

//call during setup, after join heirarchy has been formed
void Joint::calcInverseBindTransform(mat4 *parentBindTransform) 
{
    mat4 bindTransform = *parentBindTransform * *localBindTransform;
    this->inverseBindTransform = inverse(bindTransform);
    for (int i=0; i<this->children.size(); i++) {
        this->children[i]->calcInverseBindTransform(&bindTransform);
    }
}

JointTransform::JointTransform(glm::vec3 position, glm::quat rotation) 
{
    this->position = position;
    this->rotation = rotation;
}

glm::mat4 JointTransform::getLocalTransform()
{
    mat4 m;
    m = translate(mat4(1.0f), position);
    // cout << "position: " << to_string(position) << endl;
    // cout << "m1 " << to_string(m) << endl;
    m = m * toMat4(rotation);
    // cout << "m2 " << to_string(m) << endl << endl << endl;
    return m;
}

//progression [0, 1]
JointTransform JointTransform::interpolate(JointTransform frameA, JointTransform frameB, float progression)
{
    // vec3 pos = interpolate(frameA.position, frameB.position, progression);
    vec3 pos = mix(frameA.position, frameB.position, progression);
    quat rot = mix(frameA.rotation, frameB.rotation, progression);
    return JointTransform(pos, rot);
}

KeyFrame::KeyFrame(float timeStamp, std::map<std::string, JointTransform> pose) {
    this->timeStamp = timeStamp;
    this->pose = pose;
}