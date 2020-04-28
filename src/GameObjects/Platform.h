#pragma once
#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "Shape.h"
#include "Animation/AnimatedShape.h"
#include "MatrixStack.h"
#include "Hitbox.h"


class Platform : GameObject {
    public:
        Hitbox hitbox;
    protected:

};

#endif