#pragma once
#ifndef PARTICLE_H
#define PARTICLE_H

#include <string>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

class Particle
{
    public:
        glm::vec3 position;
        glm::vec3 velocity;
        float gravity;
        float life;
        float rotation;
        float scale;
        float elapsedTime = 0.0f;

        Particle(glm::vec3 position, glm::vec3 velocity, float gravity,
            float life, float rotation, float scale) 
        {
            this->position = position;
            this->velocity = velocity;
            this->gravity = gravity;
            this->life = life;
            this->rotation = rotation;
            this->scale = scale;
        }

        bool update(float dt) {
            // velocity.y += gravity * dt;
            glm::vec3 change = velocity;
            change *= dt;
            position = position + change;
            elapsedTime += dt;
            std::cout << "position:" <<  glm::to_string(position) << std::endl;
            std::cout << "velocity:" <<  glm::to_string(velocity) << std::endl;
            std::cout << "change:" <<  glm::to_string(change) << std::endl;
            return elapsedTime < life;
        }

        glm::vec3 getPosition() {
            return this->position;
        }
};

#endif