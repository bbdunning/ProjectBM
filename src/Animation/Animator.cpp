#include "Animator.h"
#include "../WindowManager.h"
#include "AnimatedShape.h"
#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;

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
    applyPoseToJoints(currentPose, entity->rootJoint, (mat4()));
}

void Animator::increaseAnimationTime() 
{
    animTime += glfwGetTime();
    if (animTime > currentAnimation->length) {
        //CHECK this
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
    cout << progression << endl;
    return interpolatePoses(frames[0], frames[1], progression);
}

void Animator::applyPoseToJoints(shared_ptr<map<string, mat4>> currentPose, Joint *joint, mat4 parentTransform)
{
    mat4 currentLocalTransform = (*currentPose)[joint->name];
    mat4 currentTransform = parentTransform * currentLocalTransform;

    //check to see if current pose is correct
    // for (map<string, mat4>::iterator it = (*currentPose).begin(); it != (*currentPose).end(); ++it) {
    //     cout << "name: " << it->first << " mat: " << to_string(it->second) << endl;
    // }
    
    // cout << joint->name << "   ";
    // cout << "currentLocal : " << to_string(currentLocalTransform) << endl;
    // cout << "currentTransform: " << to_string(currentTransform) << endl;
    // cout << "parentTransform: " << to_string(parentTransform) << endl;

    for (int i=0; i<joint->children.size();i++) {
        applyPoseToJoints(currentPose,joint->children[i], currentTransform);
    }
    //MAY NEED TO SWAP ORDER OF MULTIPLICATION
    // currentTransform = joint->inverseBindTransform * currentTransform;
    currentTransform = currentTransform * joint->inverseBindTransform ;
    joint->animatedTransform = currentTransform;
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
    for (map<string, JointTransform>::iterator it = prev.pose.begin(); it != prev.pose.end(); ++it)
        names.push_back(it->first);
    for (int i=0; i<names.size(); i++){
        JointTransform previousTransform = prev.pose[names[i]];
        JointTransform nextTransform = next.pose[names[i]];
       JointTransform currentTransform = JointTransform::interpolate(previousTransform, nextTransform, prog);
       (*currentPose)[names[i]] = currentTransform.getLocalTransform();
    }
    return currentPose;
}