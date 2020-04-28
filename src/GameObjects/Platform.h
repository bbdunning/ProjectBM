#pragma once
#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <memory>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include "../Shape.h"
#include "../Animation/AnimatedShape.h"
#include "../MatrixStack.h"
#include "../Physics/Hitbox.h"
#include "../GameObject.h"


class Platform : public GameObject {
    public:
        typedef GameObject super;
        Hitbox hitbox;
        // std::shared_ptr<GameObject> create(std::string meshPath, std::string fileName, std::string objName);
        static std::shared_ptr<Platform> create(std::string meshPath, std::string fileName, std::string objName);
    protected:

};

#endif