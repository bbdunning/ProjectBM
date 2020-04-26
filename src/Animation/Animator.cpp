#include "Animator.h"
#include "../WindowManager.h"
#include "AnimatedShape.h"
#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;

// void printPose(map<string, mat4> pose) {
//     for (map<string, mat4>::iterator it=pose.begin(); it != pose.end(); ++it) {
//         cout << "joint: " << it->first << "  mat: " << to_string(it->second) << endl;
//     }
// }
// void printPose2(map<string, JointTransform> pose) {
//     for (map<string, JointTransform>::iterator it=pose.begin(); it != pose.end(); ++it) {
//         cout << "joint: " << it->first << "  mat: " << to_string(it->second) << endl;
//     }
// }


//sets current animation
Animation::Animation(float lengthInSeconds, std::vector<KeyFrame> frames) {
    this->length = lengthInSeconds;
    this->frames = frames;
}

void Animator::doAnimation(shared_ptr<Animation> a) 
{
    this->animTime = 0;
    this->currentAnimation = a;
}

void Animator::update() 
{
    if (!entity->isAnimated)
        return;
    if (currentAnimation == nullptr) {
        return;
    }
    increaseAnimationTime();
    shared_ptr<map<string, mat4>> currentPose = calculateCurrentAnimationPose();
    mat4 temp(1.0f);
    applyPoseToJoints(currentPose, entity->rootJoint, temp);
}

void Animator::increaseAnimationTime() 
{
    animTime += glfwGetTime() / 3000.0f;
    if (animTime > currentAnimation->length) {
        animTime = fmod(animTime, currentAnimation->length);
    }
}

shared_ptr<map<string, mat4>> Animator::calculateCurrentAnimationPose()
{
    vector<KeyFrame> frames = getPreviousAndNextFrames();
    // for (int i =0; i < frames.size(); i++) {
    //     cout << frames[i].pose << endl;
    // }
    float progression = calculateProgression(frames[0], frames[1]);

    //removes inerpolation. change this once everything else works
    shared_ptr<map<string, mat4>> temp = make_shared<map<string, mat4>>();
    for (map<string, JointTransform>::iterator it=frames[0].pose.begin(); it != frames[0].pose.end(); ++it)
        (*temp)[it->first] = it->second.getLocalTransform();
    return temp;

    return interpolatePoses(frames[0], frames[1], progression);
}

void Animator::applyPoseToJoints(shared_ptr<map<string, mat4>> currentPose, Joint *joint, mat4 parentTransform)
{
    // cout << "pose size in application: " << currentPose->size() << endl;

    //eventually going to need to change this
    if (currentPose->find(joint->name) == currentPose->end()) {
        // cout << "NOT FOUND" << endl;
        (*currentPose)[joint->name] = mat4(1);
    }
    mat4 currentLocalTransform = (*currentPose)[joint->name];
    mat4 currentTransform = parentTransform * currentLocalTransform;


    //check to see if current pose is correct
    // for (map<string, mat4>::iterator it = (*currentPose).begin(); it != (*currentPose).end(); ++it) {
    //     cout << "name: " << it->first << " mat: " << to_string(it->second) << endl;
    // }
    // cout << joint->name << "   ";
    // cout << "parentTransform: " << to_string(parentTransform) << endl;
    // cout << "currentLocal : " << to_string(currentLocalTransform) << endl;
    // cout << "currentTransform: " << to_string(currentTransform) << endl;
    // cout << endl;

    for (int i=0; i<joint->children.size();i++) {
        applyPoseToJoints(currentPose, joint->children[i], currentTransform);
    }
    currentTransform = currentTransform * joint->inverseBindTransform ;
    joint->animatedTransform = currentTransform;
    // cout << joint->name << " " << to_string(joint->animatedTransform) << endl;
}

float Animator::calculateProgression(KeyFrame previousFrame, KeyFrame nextFrame) 
{
    float totalTime = nextFrame.timeStamp - previousFrame.timeStamp;
    float currentTime = animTime - previousFrame.timeStamp;
    return currentTime / totalTime;
}

vector<KeyFrame> Animator::getPreviousAndNextFrames() 
{
    KeyFrame previousFrame = currentAnimation->frames[0];
    KeyFrame nextFrame = currentAnimation->frames[0];
    for (int i=1;i<currentAnimation->frames.size();i++) {
        nextFrame = currentAnimation->frames[i];
        if (nextFrame.timeStamp > animTime)
            break;
        previousFrame = currentAnimation->frames[i];
    }
    return vector<KeyFrame> {previousFrame, nextFrame};
}

vector<string> getKeySet(map<string,JointTransform> m) 
{
    vector<string> v;
    for (map<string,JointTransform>::iterator it = m.begin(); it != m.end(); ++it) {
        v.push_back(it->first);
    }
    return v;
}


shared_ptr<std::map<std::string, glm::mat4>> Animator::interpolatePoses(KeyFrame prev, KeyFrame next, float prog)
{
    shared_ptr<map<string, mat4>> currentPose = make_shared<map<string, mat4>>();
    vector<string> names;// = getKeySet(prev.pose);
    //get all joints in pose
    for (map<string, JointTransform>::iterator it = prev.pose.begin(); it != prev.pose.end(); ++it)
        names.push_back(it->first);
    //interpolate for each joint
    for (int i=0; i<names.size(); i++){
        JointTransform previousTransform = prev.pose[names[i]];
        JointTransform nextTransform = next.pose[names[i]];
        JointTransform currentTransform = JointTransform::interpolate(previousTransform, nextTransform, prog);
        (*currentPose)[names[i]] = currentTransform.getLocalTransform();
    }
    return currentPose;
}

