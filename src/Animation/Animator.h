#pragma once
#ifndef ANIMATOR_H
#define ANIMATOR_H
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
#include "Joint.h"

class AnimatedShape;

class Animation 
{
public:
    float length; //in seconds
    std::vector<KeyFrame> frames;
    
    Animation(float lengthInSeconds, std::vector<KeyFrame> frames);
};


//sets global Joint transforms from animation data
class Animator 
{
public:
    AnimatedShape *entity;
    Animation *currentAnimation;
    float animTime;

    void update();
    void doAnimation(Animation *a);
    void increaseAnimationTime();
    void applyPoseToJoints(std::shared_ptr<std::map<std::string, glm::mat4>> currentPose, Joint *joint, glm::mat4 parentTransform);
    std::shared_ptr<std::map<std::string, glm::mat4>> calculateCurrentAnimationPose();
    float calculateProgression(KeyFrame previousFrame, KeyFrame nextFrame);
    std::vector<KeyFrame> getPreviousAndNextFrames();
    std::shared_ptr<std::map<std::string, glm::mat4>> interpolatePoses(KeyFrame prev, KeyFrame next, float prog);
};

#endif