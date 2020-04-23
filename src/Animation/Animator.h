#pragma once
#ifndef ANIMATOR_H
#define ANIMATOR_H
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
// #include "AnimatedShape.h"


class Animation 
{
public:
    float length; //in seconds
    // std::vector<KeyFrame> frames;
    std::map<std::string, KeyFrame> frames;
    
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
};

#endif