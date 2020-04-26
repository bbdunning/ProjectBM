#include "Joint.h"
#include "Animator.h"
#include <glm/gtx/quaternion.hpp>

using namespace glm;
using namespace std;

Joint::Joint(int index, std::string name, glm::mat4 localBindTransform)
{
    this->index = index;
    this->name = name;
    this->localBindTransform = localBindTransform;
    animatedTransform;
}

//call during setup, after join heirarchy has been formed
void Joint::calcInverseBindTransform(mat4 *parentBindTransform) 
{
    // mat4 bindTransform = *parentBindTransform * localBindTransform;
    // this->inverseBindTransform = inverse(bindTransform);
    // for (int i=0; i<this->children.size(); i++) {
    //     this->children[i]->calcInverseBindTransform(&bindTransform);
    // }
    this->inverseBindTransform = localBindTransform;
    for (int i=0; i<this->children.size(); i++) {
        this->children[i]->calcInverseBindTransform(parentBindTransform);
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
    m = m * toMat4(rotation);
    return m;
}

//progression [0, 1]
JointTransform JointTransform::interpolate(JointTransform frameA, JointTransform frameB, float progression)
{
    // vec3 pos = interpolate(frameA.position, frameB.position, progression);
    vec3 pos = mix(frameA.position, frameB.position, progression);
    quat rot = slerp(frameA.rotation, frameB.rotation, progression);
    cout << progression << endl;
    return JointTransform(pos, rot);
}

KeyFrame::KeyFrame(float timeStamp, std::map<std::string, JointTransform> pose) {
    this->timeStamp = timeStamp;
    this->pose = pose;
}


//adds children to Joints
void buildJointHeirarchy(shared_ptr<map<string, unsigned int>> jointMap, shared_ptr<vector<Joint>> &joints, aiNode *node, const aiScene* scene) {
    for (int i=0; i< node->mNumChildren; i++) {
        if (jointMap->find(node->mChildren[i]->mName.C_Str()) != jointMap->end()) {
            if ((*joints)[(*jointMap)[node->mName.C_Str()]].name != (*joints)[(*jointMap)[node->mChildren[i]->mName.C_Str()]].name) {
                // cout << (*jointMap)[node->mName.C_Str()] << endl;
                // (*joints)[(*jointMap)[node->mName.C_Str()]->index] = ;
                (*joints)[(*jointMap)[node->mName.C_Str()]].children.push_back(&(*joints)[(*jointMap)[node->mChildren[i]->mName.C_Str()]]);
                cout << (*joints)[(*jointMap)[node->mName.C_Str()]].name << " < " << (*joints)[(*jointMap)[node->mChildren[i]->mName.C_Str()]].name << endl;
                // (*jointMap)[node->mName.C_Str()]->children.push_back((*jointMap)[node->mChildren[i]->mName.C_Str()]);
                // &(*jointMap)[node->mChildren[i]->mName.C_Str()];
            }
        }
        buildJointHeirarchy(jointMap, joints, node->mChildren[i], scene);
    }
}

Joint* getRootJoint(shared_ptr<map<string, unsigned int>> jointMap, shared_ptr<vector<Joint>> &joints, aiNode *node) {
    if (node == nullptr || jointMap == nullptr) {
        return nullptr;
    }
    if (jointMap->find(node->mName.C_Str()) != jointMap->end()) {
        return &(*joints)[(*jointMap)[node->mName.C_Str()]];
    }
    for (int i=0; i<node->mNumChildren; i++) {
        cout << "node name: " << node->mChildren[i]->mName.C_Str() << endl;
        string childname = node->mChildren[i]->mName.C_Str();
        if (childname == "Armature"){
            return getRootJoint(jointMap, joints, node->mChildren[i]);
        }
    }
    return nullptr;
}

void printJoints(Joint *j)
{
    if (j == nullptr) {
        cout << "nothing" << endl;
        return;
    }
    cout << "joint name: " << j->name << endl;
    // cout << "local bind transform: " << to_string(j->localBindTransform) << endl;
    for (int i=0;i<j->children.size();i++)
        cout << "child: " << j->children[i]->name << endl;
    for (int i=0;i<j->children.size();i++) {
        printJoints((j->children[i]));
    }
}

void printAllJoints(vector<Joint> joints) {
for (int i=0;i<joints.size();i++) {
    cout << joints[i].name << endl;
    cout << to_string(joints[i].animatedTransform) << endl;
    }
}

void printTransforms(vector<mat4> t) {
    for (int i=0; i<t.size();i++) {
        cout << to_string(t[i]) << endl;
    }
}
