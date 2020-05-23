#pragma once
#ifndef ACTION_H
#define ACTION_H
#include <memory>
#include <map>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Phase
{
public:
    int animationNum;
};

class Action 
{
public:
    float TimeStart;
    std::map<float, Phase> phases;
};


#endif